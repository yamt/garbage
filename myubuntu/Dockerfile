FROM ubuntu:20.04 as base

ENV DEBIAN_FRONTEND="noninteractive"

# net-tools: netstat, ifconfig
# iputils-ping: ping
RUN apt-get update && apt-get install -y \
    --no-install-recommends \
    curl \
    iptables \
    make \
    m4 \
    zsh \
    pax \
    tmux \
    vim \
    less \
    iputils-ping \
    dnsutils \
    iproute2 \
    net-tools \
    ca-certificates \
    locales \
    && rm -rf /var/lib/apt/list/*
RUN sed -i -e '/en_US.UTF-8/s/^# *//' /etc/locale.gen
RUN dpkg-reconfigure locales
RUN chsh -s /usr/bin/zsh root

FROM base as install

# Note: this is amd64 binary
RUN curl -L https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-16/wasi-sdk-16.0-linux.tar.gz | (mkdir -p /opt && cd /opt && pax -rvz)
RUN ln -s wasi-sdk-16.0 /opt/wasi-sdk

# qemu: Ninja, pkg-config, libglib2.0-dev
# bsdmailutils includes hexdump
RUN apt-get update && apt-get install -y \
    --no-install-recommends \
    autoconf \
    automake \
    libtool \
    git \
    clang-11 \
    lld-11 \
    gcc \
    ninja-build \
    pkg-config \
    libglib2.0-dev \
    golang \
    bsdmainutils \
    cmake \
    wabt \
    && rm -rf /var/lib/apt/list/*

FROM install as config

RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-11 50
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-11 50
RUN update-alternatives --install /usr/bin/ld.lld ld.lld /usr/bin/ld.lld-11 50

COPY dot-files/tmux.conf /root/.tmux.conf
COPY dot-files/zshrc /root/.zshrc
COPY apt/sources.list /etc/apt/sources.list

WORKDIR /work
CMD [ "/usr/bin/zsh" ]
