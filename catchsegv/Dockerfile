FROM ubuntu:focal

RUN apt-get update && apt-get install -y clang

COPY . .
# Note: glibc backtrace uses dladdr(), which uses .dynsym section (-rdynamic)
#
# Note: Explictly ask unwinding info. (-funwind-tables)
# It's enabled by default for some platforms. (eg. amd64)
# It isn't for others. (eg. arm64)
RUN cc -o test -g -funwind-tables -rdynamic broken.c

CMD catchsegv ./test
