/Volumes/PortableSSD/llvm/build/bin/clang -S -emit-llvm --target=x86_64-apple-darwin21.6.0  -Os -Xclang -fmerge-functions -mllvm -print-before-all a.c 2>log
