
set -e

SDK=$(xcrun --sdk macosx --show-sdk-path)

sw_vers
uname -a

# homebrew-installed llvm
#CC=/usr/local/Cellar/llvm@12/12.0.1_1/bin/clang
#CC=/usr/local/Cellar/llvm/13.0.1/bin/clang
CC=/usr/local/opt/llvm@13/bin/clang
#CC=${HOME}/llvm/bin/clang
#CC=${HOME}/git/llvm-project/b/bin/clang
${CC} --version
${CC} -isysroot ${SDK} -fsanitize=address a.c
export ASAN_OPTIONS=detect_leaks=1

./a.out ipv4.google.com https
./a.out www.google.com https
./a.out ipv6.google.com https
