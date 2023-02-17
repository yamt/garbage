xcode clang
```shell
spacetanuki% cc --version
Apple clang version 14.0.0 (clang-1400.0.29.202)
Target: x86_64-apple-darwin21.6.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
spacetanuki% cc a.c b.c
Undefined symbols for architecture x86_64:
  "_nonexist", referenced from:
      _main in a-3e989c.o
      _f in b-7bc33c.o
ld: symbol(s) not found for architecture x86_64
clang: error: linker command failed with exit code 1 (use -v to see invocation)
spacetanuki%
```

wasi-sdk clang
```shell
spacetanuki% /opt/wasi-sdk-20.0+threads/bin/clang --version
clang version 16.0.0 (https://github.com/llvm/llvm-project 434575c026c81319b393f64047025b54e69e24c2)
Target: wasm32-unknown-wasi
Thread model: posix
InstalledDir: /opt/wasi-sdk-20.0+threads/bin
spacetanuki% /opt/wasi-sdk-20.0+threads/bin/clang a.c b.c  
spacetanuki% toywasm --wasi a.out
0
0
spacetanuki% 
```

a more recent version of llvm i happened to have
```
spacetanuki% ~/llvm/bin/clang --version
clang version 17.0.0 (https://github.com/llvm/llvm-project.git 6548866eb2ef3ac3632a2a1b33ff1bb5749a074f)
Target: wasm32-unknown-wasi
Thread model: posix
InstalledDir: /Users/yamamoto/llvm/bin
spacetanuki% ~/llvm/bin/clang --sysroot=/opt/wasi-sdk-20.0+threads/share/wasi-sysroot a.c b.c
spacetanuki% toywasm --wasi a.out
0
0
spacetanuki%
```

maybe a wasm-specific bug?
