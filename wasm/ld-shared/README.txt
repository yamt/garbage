with
clang version 20.0.0git (https://github.com/llvm/llvm-project.git c31ac810910ac87531de636ea508abec6e29e8ff)

spacetanuki% ./test.sh
+ CC=/Volumes/PortableSSD/llvm/build/bin/clang
+ /Volumes/PortableSSD/llvm/build/bin/clang -o liba.so -nostdlib -shared -mexception-handling -fPIC -fvisibility=default -Wl,--no-entry a.c
wasm-ld: warning: creating shared libraries, with -shared, is not yet stable
wasm-ld: error: /var/folders/74/hw1sphgx0lv63q6pq_n5grw00000gn/T/a-7ab945.o: undefined symbol: __heap_base
wasm-ld: error: /var/folders/74/hw1sphgx0lv63q6pq_n5grw00000gn/T/a-7ab945.o: undefined symbol: __heap_end
wasm-ld: error: /var/folders/74/hw1sphgx0lv63q6pq_n5grw00000gn/T/a-7ab945.o: undefined symbol: __cpp_exception
wasm-ld: error: /var/folders/74/hw1sphgx0lv63q6pq_n5grw00000gn/T/a-7ab945.o: undefined symbol: __c_longjmp
clang: error: linker command failed with exit code 1 (use -v to see invocation)
spacetanuki%
