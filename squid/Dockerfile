FROM ubuntu:22.04 as base
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    squid apache2-utils && \
    rm -rf /var/lib/apt/lists/*
COPY squid-allowall.conf .
COPY squid-basicauth.conf .
COPY entry.sh .
RUN usermod -a -G tty proxy
CMD ["./entry.sh"]
