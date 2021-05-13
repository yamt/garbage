#! /bin/sh

cc -Wall -static -o out/$(uname -s)-$(uname -m) a.c
