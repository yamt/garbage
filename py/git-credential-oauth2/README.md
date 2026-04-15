# git-credential-oauth2

## what's this

git credential helpers to implement oauth 2.0.

currently only implements github.

these helpers only implement "get" and do never store credentials.
you may want to cache them by combining this with other helpers like
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

a git credential helper for oauth 2.0 web application flow ([RFC 6749])
with PKCE. ([RFC 7636])

## git-credential-oauth2-device

a git credential helper for oauth 2.0 device authorization grants ([RFC 8628])

aka "device flow"

[RFC 6749]: https://datatracker.ietf.org/doc/html/rfc6749
[RFC 7636]: https://datatracker.ietf.org/doc/html/rfc7636
[RFC 8628]: https://datatracker.ietf.org/doc/html/rfc8628
