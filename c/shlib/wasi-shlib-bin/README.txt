from https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-21/wasi-sdk-21.0-macos.tar.gz

    libc.so

from https://github.com/yamt/toywasm/tree/master/examples/libdl
via https://github.com/yamt/garbage/tree/master/c/shlib/libdl
toywasm examples/libdl 1cf3599552df6544996dbe8b852d4e62e792d1ef
wasi-sdk 21.0

    libdl.so

from https://github.com/yamt/garbage/tree/1bbc014680dad22cf8476bff5fae87231694192f/c/shlib
wasi-sdk 21.0

    main.wasi.non-pie
    main.wasi.pie
    libfoo.so
    libbar.so
    libbaz.so
