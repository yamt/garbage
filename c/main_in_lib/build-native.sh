#! /bin/sh

set -e

./clean.sh

cc -c -o main.o main.c
ar crs libmain.a main.o

cc -c -o foo.o foo.c

cc foo.o -L. -lmain
