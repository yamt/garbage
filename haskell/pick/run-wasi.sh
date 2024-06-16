#! /bin/sh

test -x pick.wasm || wasm32-wasi-ghc -o pick.wasm -O pick.hs
toywasm --wasi --wasi-dir=. -- pick.wasm list.txt
