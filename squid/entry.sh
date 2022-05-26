#! /bin/sh

set -e
if [ -n "${PROXY_USER}" ]; then
    htpasswd -c -bm /etc/squid/htpasswd "${PROXY_USER}" "${PROXY_PASS}"
	CONF=squid-basicauth.conf
else
	CONF=squid-allowall.conf
fi
exec squid -Nd1 -f ${CONF}
