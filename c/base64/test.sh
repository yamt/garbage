#! /bin/sh

set -e
set -x

DBG="-Os -Wall -Wextra -Wvla"
cc ${DBG} -o enc base64encode.c
cc ${DBG} -o dec base64decode.c

printf "hello, base64\n" | ./enc | ./dec
