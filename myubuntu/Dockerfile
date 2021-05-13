FROM ubuntu:20.04 as base

ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update && apt-get install -y \
    curl \
    iptables \
    make \
    m4 \
    zsh \
    pax \
    tmux \
    vim \
    less \
    && rm -rf /var/lib/apt/list/*

RUN chsh -s /usr/bin/zsh root

FROM base as install

# qemu: Ninja, pkg-config, libglib2.0-dev
# bsdmailutils includes hexdump
RUN apt-get update && apt-get install -y \
    git \
    clang-11 \
    ninja-build \
    pkg-config \
    libglib2.0-dev \
    golang \
    bsdmainutils \
    && rm -rf /var/lib/apt/list/*

FROM install as config

RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-11 50
RUN update-alternatives --install /usr/bin/c++ cc++ /usr/bin/clang++-11 50

COPY dot-files/tmux.conf /root/.tmux.conf
COPY dot-files/zshrc /root/.zshrc

ENTRYPOINT [ "/usr/bin/zsh" ]