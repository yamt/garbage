#! /bin/sh

# eg.
# ./list_victims.sh x 'test-.*' 60 2

set -e

PROG=$0
debug() {
    #echo "# DEBUG(${PROG}): $@" >& 2
}

if [ $# -lt 4 ]; then
    echo "usage: $0 ds snapname-regex threshold-age-in-sec min-keep"
    exit 2
fi

DS=$1
re=$2
d=$3
nkeep=$4

now=$(date +%s)
t=$((now - d))
debug $t
# note: -S creation outputs the latest snapshot first
zfs list -t snap -o name,creation -S creation -Hprd1 ${DS} | while read s c; do
    debug "checking $s $c"
    snapname=${s#*@}
    if ! echo ${snapname} | grep $re > /dev/null; then
        debug "skip $s as snapname regex didn't match"
        continue
    fi
    if [ $nkeep -gt 0 ]; then
        debug "skip $s as it's one of latest snapshots"
        nkeep=$((nkeep - 1))
        continue
    fi
    if [ $c -gt $t ]; then
        debug "skip $s as it's new"
        continue
    fi
    echo $s
done
