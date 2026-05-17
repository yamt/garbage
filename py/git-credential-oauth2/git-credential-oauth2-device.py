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
# this helper only implements "get" and does never store credentials.
# you may want to cache them by combining this with other helpers like
# osxkeychain.

# an usage example:
#
# [credential]
#     helper = cache --timeout 21600
#     helper = oauth2-device

# references:
#     https://datatracker.ietf.org/doc/html/rfc8628
#     gitcredentials(7)
#     git-credential(1)

import argparse
import webbrowser
import urllib.request
import json
import sys
import time
from collections import namedtuple

try:
    import qrcode
except ModuleNotFoundError:
    qrcode = None

Provider = namedtuple(
    "OAuthProvider",
    [
        "client_id",
        "auth_url",
        "token_url",
        "default_scope",
    ],
)

# github.com git-credential-oauth2-device
# https://docs.github.com/en/apps/oauth-apps/building-oauth-apps
# https://docs.github.com/en/apps/oauth-apps/building-oauth-apps
# https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/best-practices-for-creating-an-oauth-app#dont-enable-device-flow-without-reason
github_com_provider = Provider(
    client_id="Ov23liXxUEnSmBOA1hsz",
    auth_url="https://github.com/login/device/code",
    token_url="https://github.com/login/oauth/access_token",
    # https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/scopes-for-oauth-apps#available-scopes
    default_scope="repo workflow",
)

# gitlab.com git-credential-oauth2-device
# https://docs.gitlab.com/integration/oauth_provider/
# https://docs.gitlab.com/api/oauth2/#device-authorization-grant-flow
gitlab_com_provider = Provider(
    client_id="d86de9e91bd399ab1a1e3a050859bb24e4ca996eb1a507da853201a59188594a",
    auth_url="https://gitlab.com/oauth/authorize_device",
    token_url="https://gitlab.com/oauth/token",
    # https://docs.gitlab.com/integration/oauth_provider/#view-all-authorized-applications
    default_scope="write_repository",
)

grant_type = "urn:ietf:params:oauth:grant-type:device_code"
provider = None
scope = None


def msg(text, cont=False):
    if cont:
        print(f"{text}", file=sys.stderr)
    else:
        print(f"OAuth2 Device Flow: {text}", file=sys.stderr)


def get_token():
    data = {
        "client_id": provider.client_id,
        "scope": scope,
    }
    headers = {
        "Content-Type": "application/x-www-form-urlencoded",
        "Accept": "application/json",
    }
    encoded_data = urllib.parse.urlencode(data).encode("utf-8")
    req = urllib.request.Request(
        url=provider.auth_url, data=encoded_data, headers=headers
    )
    with urllib.request.urlopen(req) as resp:
        resp = resp.read()
        j = json.loads(resp)

    device_code = j["device_code"]
    user_code = j["user_code"]
    verification_uri = j["verification_uri"]
    verification_uri_complete = j.get("verification_uri_complete")
    if verification_uri_complete is not None:
        verification_uri = verification_uri_complete
    expires_in = j["expires_in"]
    interval = j["interval"]

    if qrcode:
        msg(f"Scan the following QR code or visit:\n\t{verification_uri}")
    else:
        msg(f"Visit:\n\t{verification_uri}")
    if verification_uri_complete is None:
        msg(f"And enter the code:\n\t{user_code}", cont=True)
    msg(f"The code expires in {expires_in} seconds.", cont=True)
    if qrcode:
        qr = qrcode.QRCode(
            version=1,
            error_correction=qrcode.constants.ERROR_CORRECT_L,
            box_size=10,
            border=4,
        )
        qr.add_data(verification_uri)
        qr.print_ascii(out=sys.stderr)

    try:
        webbrowser.get()
        msg("Trying to open the verification URI with a web browser.")
        msg("If it doesn't work, follow the above instructions manually.")
        webbrowser.open_new_tab(verification_uri)
    except:
        pass

    # https://datatracker.ietf.org/doc/html/rfc8628#section-3.4
    return get_access_token(
        {
            "device_code": device_code,
            "grant_type": grant_type,
        },
        interval,
    )


# https://datatracker.ietf.org/doc/html/rfc6749#section-6
def get_token_with_refresh_token(refresh_token):
    return get_access_token(
        {
            "grant_type": "refresh_token",
            "refresh_token": refresh_token,
            "scope": scope,
        },
        None,
    )


def get_access_token(extra_data, interval):
    headers = {
        "Content-Type": "application/x-www-form-urlencoded",
        "Accept": "application/json",
    }
    data = {
        "client_id": provider.client_id,
    }
    data.update(extra_data)
    encoded_data = urllib.parse.urlencode(data).encode("utf-8")
    while True:
        req = urllib.request.Request(
            url=provider.token_url, data=encoded_data, headers=headers
        )
        try:
            with urllib.request.urlopen(req) as resp:
                resp = resp.read()
        except urllib.error.HTTPError as e:
            # 400 is expected for errors like "authorization_pending".
            # for some reasons, github.com gives us 200 for them though.
            # https://datatracker.ietf.org/doc/html/rfc8628#section-3.5
            # https://datatracker.ietf.org/doc/html/rfc6749#section-5.2
            if e.code != 400:
                raise
            resp = e.fp.read()

        j = json.loads(resp)

        # https://datatracker.ietf.org/doc/html/rfc8628#section-3.5
        error = j.get("error")
        if error is None:
            break
        if interval is not None:
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
        msg(f"unhandled error: {error}")
        msg(f"error description: {error_description}")
        msg(f"error uri: {error_uri}")
        exit(0)

    # note: type is case insensitive.
    # https://datatracker.ietf.org/doc/html/rfc6749#section-4.2.2
    token_type = j["token_type"]
    if token_type.lower() != "bearer":
        msg(f"unknown token type: {token_type}")
        exit(0)

    # note: github doesn't give us refresh_token. github oauth
    # access tokens have no expirations.
    # note: gitlab gives us refresh_token. expires_in is typically 7200.
    access_token = j["access_token"]
    expires_in = j.get("expires_in")
    refresh_token = j.get("refresh_token")
    return access_token, expires_in, refresh_token


def recv_git_credential_parameters():
    d = {}
    for l in sys.stdin:
        l = l.strip()
        if not l:
            break
        k, v = l.split("=", 1)
        if k.endswith("[]"):
            # multi-valued attribute
            if v == "":
                # empty means to clear
                d[k] = []
            else:
                d.setdefault(k, []).append(v)
        else:
            d[k] = v
    return d


def send_git_credentail_results(d):
    for k, v in d.items():
        if isinstance(v, list):
            for i in v:
                print(f"{k}={i}")
        elif v is not None:
            print(f"{k}={v}")


def to_utc(expires_in):
    try:
        expires_in_int = int(expires_in)
    except (TypeError, ValueError):
        return None
    return int(time.time()) + expires_in_int


def main():
    global scope
    global provider

    parser = argparse.ArgumentParser()
    parser.add_argument("--scope")
    parser.add_argument("command")
    args = parser.parse_args()
    scope = args.scope
    if args.command != "get":
        exit(0)

    d = recv_git_credential_parameters()
    if d["host"] == "github.com":
        provider = github_com_provider
    elif d["host"] == "gitlab.com":
        provider = gitlab_com_provider
    else:
        exit(0)
    if scope is None:
        scope = provider.default_scope

    d.setdefault("username", "x")  # any non-empty string is ok
    refresh_token = d.pop("oauth_refresh_token", None)
    d.pop("password_expiry_utc", None)
    try:
        if refresh_token is not None:
            msg("Refreshing access token...")
            access_token, expires_in, refresh_token = get_token_with_refresh_token(
                refresh_token
            )
            msg("Successfully refreshed.")
        else:
            access_token, expires_in, refresh_token = get_token()
    except urllib.error.HTTPError as e:
        msg(f"HTTPError: {e.read().decode()}")
        exit(0)
    msg("Successfully obtained a new access token.")
    d["password"] = access_token
    if expires_in is not None:
        msg(f"Access token will expire in {expires_in} seconds.")
        d["password_expiry_utc"] = to_utc(expires_in)
    else:
        msg("Access token has no expiration.")
    d["oauth_refresh_token"] = refresh_token
    send_git_credentail_results(d)


if __name__ == "__main__":
    main()
