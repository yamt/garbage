#! /bin/sh

set -e

BINDIR=${BINDIR:-~/.local/bin}
install -v -d ${BINDIR}
for x in git-credential-oauth2-webapp git-credential-oauth2-device; do
	install -v -r -m 555 "$@" $x.py ${BINDIR}/$x
done
