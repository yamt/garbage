#! /bin/sh

set -e
set -x

LIBDL=

case $(uname -s) in
Darwin)
    # macos two-level namespace
    CLINKFLAGS="${CLINKFLAGS} -Xlinker -undefined -Xlinker dynamic_lookup"

    # macos flat namespace
    #CLINKFLAGS="-Xlinker -flat_namespace"
    #CLINKFLAGS="${CLINKFLAGS} -Xlinker -undefined -Xlinker suppress"
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

cc -fPIC ${CLINKFLAGS} -shared -o libbar.so bar.c
# note: link libbar.so explicitly so that func_in_foo() calls
# func_in_bar@libbar with two level namespace.
# also, this should ensure the ctor calling order as bar -> foo -> main.
cc -fPIC ${CLINKFLAGS} -shared -o libfoo.so foo.c libbar.so
cc -fPIC ${CLINKFLAGS} -shared -o libbaz.so baz.c
cc -fPIC ${CLINKFLAGS} -o main main.c main2.c libfoo.so libbar.so ${LIBDL}

./main
