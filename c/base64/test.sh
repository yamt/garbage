#! /bin/sh

set -e
set -x

DBG="-Os -Wall -Wextra -Wvla"
cc ${DBG} -DTEST -o enc base64encode.c
cc ${DBG} -DTEST -o dec base64decode.c

printf "hello, base64\n" | ./enc | ./dec

T=$(mktemp)
T2=$(mktemp)
dd if=/dev/urandom of=${T} count=100
< ${T} ./enc | ./dec > ${T2}
cmp ${T} ${T2}
rm ${T} ${T2}
