
uname -a

# homebrew-installed llvm
CC=/usr/local/Cellar/llvm@12/12.0.1_1/bin/clang
#CC=/usr/local/Cellar/llvm/13.0.1/bin/clang
${CC} --version
${CC} -fsanitize=address a.c
export ASAN_OPTIONS=detect_leaks=1

# leak
./a.out ipv4.google.com https

# no leak
#./a.out www.google.com https

# no leak
#./a.out ipv6.google.com https
