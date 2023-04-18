CC=/opt/wasi-sdk-20.0/bin/clang
WASMTOOLS=${HOME}/git/wasm/wasm-tools/target/release/wasm-tools
HOST=${HOME}/git/wasm/preview2-prototyping/target/release/host
ADAPTER=${HOME}/git/wasm/preview2-prototyping/target/wasm32-unknown-unknown/release/wasi_snapshot_preview1.wasm

${CC} a.c
${WASMTOOLS} component new a.out -o cstat.wasm --adapt ${ADAPTER}

echo "===== host cli"
${HOST} cstat.wasm

echo "===== toywasm"
toywasm --wasi a.out

echo "===== wasmtime"
wasmtime run a.out

echo "===== iwasm"
iwasm a.out
