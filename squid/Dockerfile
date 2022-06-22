FROM alpine:3.16 as base
RUN apk add \
    squid \
    apache2-utils \
    shadow
COPY squid-allowall.conf .
COPY squid-basicauth.conf .
COPY entry.sh .
RUN usermod -a -G tty squid
CMD ["./entry.sh"]
