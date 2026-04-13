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

# a git credential helper for oauth 2.0 device authorization grants (RFC 8628)
# aka "device flow"
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
#     helper = deviceflow

# references:
#     https://datatracker.ietf.org/doc/html/rfc8628
#     https://docs.github.com/en/apps/oauth-apps/building-oauth-apps
#     https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/best-practices-for-creating-an-oauth-app#dont-enable-device-flow-without-reason
#     gitcredentials(7)
#     git-credential(1)
#     git-credential-cache(1)

import requests
import json
import qrcode
import sys
import time

client_id = "Ov23liXxUEnSmBOA1hsz"  # git-credential-deviceflow

url = "https://github.com/login/device/code"
token_url = "https://github.com/login/oauth/access_token"
grant_type = "urn:ietf:params:oauth:grant-type:device_code"

# https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/scopes-for-oauth-apps#available-scopes
scope = "repo"


def get_token():
    s = requests.Session()
    data = {
        "client_id": client_id,
        "scope": scope,
    }
    headers = {
        "Accept": "application/json",
    }
    resp = s.post(url, data=data, headers=headers)
    resp.raise_for_status()

    j = json.loads(resp.content)

    device_code = j["device_code"]
    user_code = j["user_code"]
    verification_uri = j["verification_uri"]
    expires_in = j["expires_in"]
    interval = j["interval"]

    f = sys.stderr
    print(f"scan the following QR code or visit:\n{verification_uri}\n", file=f)
    print(f"and enter the code:\n{user_code}\n", file=f)
    print(f"the code expires in {expires_in} seconds", file=f)
    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.constants.ERROR_CORRECT_L,
        box_size=10,
        border=4,
    )
    qr.add_data(verification_uri)
    qr.print_ascii(out=f)

    # https://datatracker.ietf.org/doc/html/rfc8628#section-3.4
    data = {
        "client_id": client_id,
        "device_code": device_code,
        "grant_type": grant_type,
    }
    while True:
        resp = s.post(token_url, data=data, headers=headers)
        resp.raise_for_status()

        j = json.loads(resp.content)
        # print(json.dumps(j, indent=4))

        # https://datatracker.ietf.org/doc/html/rfc8628#section-3.5
        error = j.get("error")
        if error is None:
            break
        if error == "authorization_pending":
            time.sleep(interval)
            continue
        if error == "slow_down":
            interval += 5
            time.sleep(interval)
            continue
        # access_denied, expired_token, device_flow_disabled, etc
        error_description = j.get("error_description")
        error_uri = j.get("error_uri")
        print(f"unhandled error: {error}", file=f)
        print(f"error description: {error_description}", file=f)
        print(f"error uri: {error_uri}", file=f)
        exit(1)

    token_type = j["token_type"]
    if token_type != "bearer":
        print(f"unknown token type: {token_type}", file=f)
        exit(1)

    # note: github doesn't give us refresh_token.
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


cmd = sys.argv[1]
if cmd != "get":
    exit(1)

d = recv_git_credential_parameters()
if d["host"] != "github.com":
    exit(1)
d["username"] = "x"  # any non-empty string is ok
d["password"] = get_token()
send_git_credentail_results(d)
