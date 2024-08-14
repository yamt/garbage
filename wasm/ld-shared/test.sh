#! /bin/sh

set -e
set -x

CC=${CC:-/Volumes/PortableSSD/llvm/build/bin/clang}

${CC} -o liba.so -nostdlib -shared -mexception-handling -fPIC -fvisibility=default -Wl,--no-entry a.c
${CC} -o libb.so -nostdlib -shared -fPIC -fvisibility=default -Wl,--no-entry b.c liba.so
