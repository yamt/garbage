#! /bin/sh

# toywasm
../../listenexec/listenexec -p 9999 ~/git/toywasm/b.thread/toywasm --wasi --wasi-dir=. -- httpd.wasm "$@"

# wasm on wasm
# ../../listenexec/listenexec -p 9999 ~/git/toywasm/b.thread/toywasm --wasi --wasi-dir=. --wasi-dir=${HOME}/git/toywasm/build.wasm -- ${HOME}/git/toywasm/build.wasm/toywasm --wasi -- httpd.wasm

# iwasm
# ../../listenexec/listenexec -p 9999 ~/git/wasm-micro-runtime/product-mini/platforms/darwin/b.thread/iwasm --max-threads=16 --dir=. httpd.wasm

# wasmtime
# ../../listenexec/listenexec -s -p 9999 wasmtime --listenfd --dir=. httpd.wasm
# ../../listenexec/listenexec -s -p 9999 wasmtime --wasm-features=threads --wasi-modules=experimental-wasi-threads --listenfd --dir=. httpd.wasm
# note: wasmtime bails out if we give it a socket as stdin

# eg.
# ab -c5 -n10000 "http://[::1]:9999/Makefile"
