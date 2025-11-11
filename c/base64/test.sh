#! /bin/sh

set -e
set -x

DBG="-Os -Wall -Wextra -Wvla"
cc ${DBG} -DTEST -o enc base64encode.c
cc ${DBG} -DTEST -o dec base64decode.c

printf "hello, base64\n" | ./enc | ./dec
