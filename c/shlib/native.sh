#! /bin/sh

set -e
set -x

case $(uname -s) in
Darwin)
    # macos two-level namespace
    CLINKFLAGS="${CLINKFLAGS} -Xlinker -undefined -Xlinker dynamic_lookup"

    # macos flat namespace
    #CLINKFLAGS="-Xlinker -flat_namespace"
    #CLINKFLAGS="${CLINKFLAGS} -Xlinker -undefined -Xlinker suppress"
    ;;
*)
    # elf
    CLINKFLAGS="-Xlinker -rpath=."
    ;;
esac

cc -fPIC ${CLINKFLAGS} -shared -o libbar.so bar.c
cc -fPIC ${CLINKFLAGS} -shared -o libfoo.so foo.c libbar.so
cc -fPIC ${CLINKFLAGS} -o main main.c main2.c libfoo.so libbar.so

./main
