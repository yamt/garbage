# git-credential-oauth2

## what's this

git credential helpers to implement oauth 2.0.

currently only implements github.

## `~/.gitconfig` example

```
[credential]
    # empty helper overrides the system default (eg. osxkeychain)
    helper =
    helper = cache --timeout 21600
    helper = oauth2-webapp
    helper = oauth2-device
```

## git-credential-oauth2-webapp

a git credential helper for oauth 2.0 web application flow (RFC 6749)

## git-credential-oauth2-device

a git credential helper for oauth 2.0 device authorization grants (RFC 8628)

aka "device flow"
