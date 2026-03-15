#! /bin/sh

# expected usage:
#
# ./mon.sh 192.168.75.50 192.168.75.51 2>&1 | tee log
# <leave it for days>
# ^C
#
# you may want to use a file to list targets:
#
# ./mon.sh $(sed -e '/^#/d' -e '/^$/d' targets.txt)
#
# analyze the log:
#
# grep "^LOG:" log
# awk '$1 == "LOG:" {print $2, $3, $4}' log

set -e

TARGETS="$@"

UNAME=$(uname -s)

case ${UNAME} in
Darwin|FreeBSD)
    PING="ping -c1 -t5"
    ;;
NetBSD|Linux)
    # XXX assume iputils for linux
    PING="ping -c1 -w5"
    ;;
esac

log() {
    TS=$(date +%s)
    echo "LOG: ${TS} $@"
}

while :; do
    for t in ${TARGETS}; do
        if ${PING} ${t}; then
            log ${t} 1
        else
            log ${t} 0
        fi
    done
    sleep 1
done
