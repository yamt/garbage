#! /bin/sh

cc -Os -shared -o lib.so lib.c
cc -Os a.c lib.so
./a.out
