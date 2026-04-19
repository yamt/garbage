#! /usr/bin/env python3

# Copyright (c)2026 YAMAMOTO Takashi,
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

# a git credential helper for oauth 2.0 web application flow (RFC 6749)
#
# currently only implements github.
#
# this helper only implements "get" and does never store credentials.
# you may want to cache them by combining this with other helpers like
# osxkeychain.

# an usage example:
#
# [credential]
#     helper = cache --timeout 21600
#     helper = oauth2-webapp

# references:
#     https://docs.github.com/en/apps/oauth-apps/building-oauth-apps
#     https://datatracker.ietf.org/doc/html/rfc6749

import argparse
import webbrowser
import http.server
import base64
import urllib
import urllib.request
import hashlib
import json
import secrets
import sys
import time

client_id = "Ov23li9YAYcNhPanUtBB"  # git-credential-oauth2-webapp

# note: public applications can't hold the credential securely.
# we simply hardcode it here.
client_secret = "55eaf54f333bf6eec0a12297af12679eff2ae4eb"

auth_url = "https://github.com/login/oauth/authorize"
token_url = "https://github.com/login/oauth/access_token"
redirect_address = ("127.0.0.1", 0)

# https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/scopes-for-oauth-apps#available-scopes
scope = None

state = None
httpd = None
code = None


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        # note: any local users can inject anything into this endpoint.
        # verify state to ensure it's ours.
        result = "success"
        response = 200
        q = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query)
        lcode = q.get("code", [None])[0]
        if lcode is None:
            result = "no code"
            response = 403
        elif state != q.get("state", [None])[0]:
            # https://datatracker.ietf.org/doc/html/rfc6749#section-10.12
            result = "state missmatch"
            response = 403
        self.send_response(response)
        self.send_header("Content-type", "text/html; charset=ascii")
        self.end_headers()
        self.wfile.write(f"git-credential-oauth2-webapp: {result}".encode("ascii"))
        if response == 200:
            global code
            code = lcode

    # disable request logging as it contains the redirected code
    def log_message(self, *args):
        pass


def start_local_httpd():
    global httpd
    httpd = http.server.HTTPServer(redirect_address, Handler)
    # note: github seems to allow port number differecnes from
    # the uri registered in the application settings.
    return f"http://{redirect_address[0]}:{httpd.server_port}/"


def get_code():
    while code is None:
        httpd.handle_request()
    return code


def get_token():
    global state
    state = secrets.token_urlsafe(128)

    local_httpd_url = start_local_httpd()

    # https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/authorizing-oauth-apps#1-request-a-users-github-identity
    # note about verifier length:
    #  - 43-128 characters according to RFC 7636.
    #  - github token_url returns 400 Bad Request for >96 characters.
    pkce_verifier = secrets.token_urlsafe(96)
    pkce_hash = hashlib.sha256(pkce_verifier.encode("utf-8")).digest()
    pkce_hash = base64.urlsafe_b64encode(pkce_hash).decode("utf-8")
    pkce_hash = pkce_hash.replace("=", "")
    params = {
        "client_id": client_id,
        "response_type": "code",
        "state": state,
        "scope": scope,
        "redirect_uri": local_httpd_url,
        "code_challenge": pkce_hash,
        "code_challenge_method": "S256",
    }
    params = urllib.parse.urlencode(params)
    url = f"{auth_url}?{params}"
    webbrowser.open(url)

    code = get_code()

    # https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/authorizing-oauth-apps#2-users-are-redirected-back-to-your-site-by-github
    data = {
        "client_id": client_id,
        "client_secret": client_secret,
        "code": code,
        "redirect_uri": local_httpd_url,
        "code_verifier": pkce_verifier,
    }
    headers = {
        "Content-Type": "application/x-www-form-urlencoded",
        "Accept": "application/json",
    }
    encoded_data = urllib.parse.urlencode(data).encode("utf-8")
    req = urllib.request.Request(url=token_url, data=encoded_data, headers=headers)
    with urllib.request.urlopen(req) as resp:
        resp = resp.read()
        j = json.loads(resp)

    f = sys.stderr

    error = j.get("error")
    if error is not None:
        print(f"error {error}", file=f)
        print(f"full response:\n{json.dumps(j, indent=4)}", file=f)
        exit(1)

    token_type = j["token_type"]
    if token_type != "bearer":
        print(f"unknown token type: {token_type}", file=f)
        exit(1)

    # note: github doesn't give us refresh_token. github oauth
    # access tokens have no expirations.
    access_token = j["access_token"]
    return access_token


def recv_git_credential_parameters():
    d = {}
    for l in sys.stdin:
        l = l.strip()
        if not l:
            break
        k, v = l.split("=", 1)
        d[k] = v
    return d


def send_git_credentail_results(d):
    for k, v in d.items():
        print(f"{k}={v}")


# quick check
try:
    webbrowser.get()
except:
    exit(0)


parser = argparse.ArgumentParser()
parser.add_argument("--scope", default="repo")
parser.add_argument("command")
args = parser.parse_args()
scope = args.scope
if args.command != "get":
    exit(1)

d = recv_git_credential_parameters()
if d["host"] != "github.com":
    exit(0)
d["username"] = "x"  # any non-empty string is ok
d["password"] = get_token()
send_git_credentail_results(d)
