FROM ubuntu:focal

RUN apt-get update && apt-get install -y clang

COPY . .
RUN cc -o test -g -rdynamic broken.c

CMD catchsegv ./test
