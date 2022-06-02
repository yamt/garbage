#! /bin/sh

B=$(readlink "$0" || echo "$0")
D=$(cd ${B%/*} && pwd -P)
ioreg -a -c IOUserSerial -r -l | python3 $D/find_serial.py "$@"
