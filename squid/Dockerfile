FROM ubuntu:22.04 as base
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update && \
    apt-get install -y --no-install-recommends squid && \
    rm -rf /var/lib/apt/lists/*
COPY squid.conf /etc/squid/squid.conf
RUN usermod -a -G tty proxy
CMD ["squid","-Nd1"]
