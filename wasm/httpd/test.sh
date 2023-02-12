#! /bin/sh

# toywasm
../../listenexec/listenexec -p 9999 ~/git/toywasm/b.thread/toywasm --wasi --wasi-dir=. -- httpd.wasm "$@"

# wasm on wasm
# ../../listenexec/listenexec -p 9999 ~/git/toywasm/b.thread/toywasm --wasi --wasi-dir=. --wasi-dir=${HOME}/git/toywasm/build.wasm -- ${HOME}/git/toywasm/build.wasm/toywasm --wasi -- httpd.wasm

# iwasm
# ../../listenexec/listenexec -p 9999 ~/git/wasm-micro-runtime/product-mini/platforms/darwin/b.thread/iwasm --max-threads=16 --dir=. httpd.wasm

# eg.
# ab -c5 -n10000 "http://[::1]:9999/Makefile"
