#! /bin/sh

set -e
set -x

# XXX for some reasons, this takes very long on my environment.
# ../myconfig.sh  14.46s user 25.44s system 16% cpu 3:59.58 total

exec ../configure \
--static \
--target-list=aarch64-linux-user,arm-linux-user \
--without-default-features \
--enable-linux-user \
--disable-pie \
--disable-vnc \
"$@"

