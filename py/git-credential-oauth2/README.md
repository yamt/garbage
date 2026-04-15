# git-credential-oauth2

## What's this

Git credential helpers to implement OAuth 2.0.

Currently only implements github.

These helpers only implement "get" and do never store credentials.
You may want to cache them by combining this with other helpers like
`osxkeychain`.

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

## git-credential-oauth2-webapp

A git credential helper for OAuth 2.0 web application flow ([RFC 6749])
with PKCE. ([RFC 7636])

## git-credential-oauth2-device

A git credential helper for OAuth 2.0 device authorization grants, ([RFC 8628])
aka "device flow".

[RFC 6749]: https://datatracker.ietf.org/doc/html/rfc6749
[RFC 7636]: https://datatracker.ietf.org/doc/html/rfc7636
[RFC 8628]: https://datatracker.ietf.org/doc/html/rfc8628
