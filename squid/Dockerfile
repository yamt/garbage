FROM ubuntu:22.04 as base
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    squid apache2-utils && \
    rm -rf /var/lib/apt/lists/*
COPY squid.conf /etc/squid/squid.conf
RUN usermod -a -G tty proxy
RUN htpasswd -c -bm /etc/squid/htpasswd aaa bbb
CMD ["squid","-Nd1"]
