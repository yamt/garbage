#! /bin/sh

TOYWASM=${TOYWASM:-${TEST_RUNTIME_EXE:-toywasm}}

set -x
set -e

${TOYWASM} --wasi \
--dyld \
--dyld-dlfcn \
--dyld-path . \
main.wasi.pie

${TOYWASM} --wasi \
--dyld \
--dyld-dlfcn \
--dyld-path . \
main.wasi.non-pie
