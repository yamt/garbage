# git-credential-oauth2

## What's this

Git credential helpers to implement OAuth 2.0, written in python.

Currently only implements github.

These helpers only implement "get" and do never store credentials.
You may want to cache them by combining this with other helpers like
`osxkeychain`.

### git-credential-oauth2-webapp

A git credential helper for OAuth 2.0 web application flow ([RFC 6749])
with PKCE. ([RFC 7636])

You may use this for environments with a local web browser.

### git-credential-oauth2-device

A git credential helper for OAuth 2.0 device authorization grants, ([RFC 8628])
aka "device flow".

You may use this for environments where you can read its `stderr` output,
including the verification uri / QR code and `user code`.
Although you still need a web browser, it doesn't need to be in the same
device as git cli.

#### Optional dependency

If python [qrcode] module is available, this helper outputs the QR code
of the verification uri as well.

You may install it as:
```shell
pkgin in py313-qrcode
```

[RFC 6749]: https://datatracker.ietf.org/doc/html/rfc6749
[RFC 7636]: https://datatracker.ietf.org/doc/html/rfc7636
[RFC 8628]: https://datatracker.ietf.org/doc/html/rfc8628

[qrcode]: https://github.com/lincolnloop/python-qrcode

## install/uninstall

Install destination is '~/.local/bin' by default.
It can be overridden by setting `BINDIR` environment variable.

### Install
```shell
./install.sh
```

### Install to a non-default location
```shell
BINDIR=~/bin ./install.sh
```

### Install with symlink (for developers)
```shell
./install.sh -l a
```

### Uninstall
```shell
./uninstall.sh
```

## `~/.gitconfig` example

```
[credential]
    # empty helper overrides the system default (eg. osxkeychain)
    helper =
    helper = cache --timeout 21600
    # if web application flow didn't work (eg. web browser is not available)
    # fallback to device flow
    helper = oauth2-webapp
    helper = oauth2-device
```

## Scope

The default scope is 'repo'.

You can override it as the following:
```
    helper = oauth2-device --scope public_repo
```

See [Scopes for OAuth apps] for available scopes for github.

[Scopes for OAuth apps]: https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/scopes-for-oauth-apps
