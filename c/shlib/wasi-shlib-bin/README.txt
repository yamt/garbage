from https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-21/wasi-sdk-21.0-macos.tar.gz

    libc.so

from https://github.com/yamt/toywasm/tree/master/examples/libdl
via https://github.com/yamt/garbage/tree/master/c/shlib/libdl
toywasm examples/libdl 1cf3599552df6544996dbe8b852d4e62e792d1ef
wasi-sdk 21.0

    libdl.so

from https://github.com/yamt/garbage/tree/58d93e5af394cb1c748f4ac86ca6829f65d4ea77/c/shlib
wasi-sdk 24.0 sysroot
llvm c34cba0413d1c6968e4b5d423295011f93e4c596

    main.wasi.non-pie
    main.wasi.pie
    libfoo.so
    libbar.so
    libbaz.so
