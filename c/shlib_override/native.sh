#! /bin/sh

set -e
set -x

CC=cc
AR=ar

CC="${CC} -fPIC"

# static build
${CC} -Os -c -o lib1-native.o lib1.c
${CC} -Os -c -o lib2-native.o lib2.c
rm -f lib-native.a
${AR} crs lib-native.a lib1-native.o lib2-native.o
${CC} -Os -o native.static main.c lib-native.a

LIBDL=

case $(uname -s) in
Darwin)
    # macos two-level namespace
    #CLINKFLAGS="${CLINKFLAGS} -Xlinker -undefined -Xlinker dynamic_lookup"

    # macos flat namespace
    CLINKFLAGS="-Xlinker -flat_namespace"
    CLINKFLAGS="${CLINKFLAGS} -Xlinker -undefined -Xlinker suppress"
    ;;
Linux)
    CLINKFLAGS="-Xlinker -rpath=."
    LIBDL=-ldl
    ;;
*)
    # elf
    CLINKFLAGS="-Xlinker -rpath=."
    ;;
esac

${CC} -Os ${CLINKFLAGS} -shared -fvisibility=default -o lib-native.so lib1.c lib2.c
${CC} -Os ${CLINKFLAGS} -fvisibility=default -o native main.c lib-native.so

# this works
./native.static
./native
