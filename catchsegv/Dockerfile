FROM ubuntu:focal

RUN apt-get update && apt-get install -y clang

COPY . .
# Note: glibc backtrace uses dladdr(), which uses .dynsym section
RUN cc -o test -g -rdynamic broken.c

CMD catchsegv ./test
