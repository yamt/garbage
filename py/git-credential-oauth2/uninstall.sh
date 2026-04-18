#! /bin/sh

set -e

BINDIR=${BINDIR:-~/.local/bin}
for x in git-credential-oauth2-webapp git-credential-oauth2-device; do
	rm -f ${BINDIR}/$x
done
rmdir ${BINDIR} || :
