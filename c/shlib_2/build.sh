#! /bin/sh

set -e
set -x

CC=/Volumes/PortableSSD/llvm/build/bin/clang
#CC=/Volumes/PortableSSD/git/wasi-sdk/build/toolchain/install/bin/clang
#CC=${CC:-/opt/wasi-sdk-24.0/bin/clang}

${CC} -Os -ffreestanding -nostdlib -fPIC -shared -fvisibility=default -Xlinker --no-entry -S -o lib.s lib.c
${CC} -v -Os -ffreestanding -nostdlib -fPIC -shared -fvisibility=default -Xlinker --no-entry -o lib.so lib.c

${CC} -Os -ffreestanding -nostdlib -fPIC \
-Xlinker -Bdynamic \
-Xlinker --export-table \
-Xlinker --growable-table \
-Xlinker --export-memory \
-Xlinker --export=__stack_pointer \
-Xlinker --export=__heap_base \
-Xlinker --export=__heap_end \
-Xlinker --entry=_start -S -o main.s main.c

${CC} -Os -ffreestanding -nostdlib -fPIC \
-v \
-Xlinker -Bdynamic \
-Xlinker --export-table \
-Xlinker --growable-table \
-Xlinker --export-memory \
-Xlinker --export=__stack_pointer \
-Xlinker --export=__heap_base \
-Xlinker --export=__heap_end \
-Xlinker --entry=_start -o main main.c lib.so

exit 0
-Xlinker --unresolved-symbols=import-dynamic \
-Xlinker -mllvm \
-Xlinker -debug \
