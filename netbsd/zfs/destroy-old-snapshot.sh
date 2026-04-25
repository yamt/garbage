#! /bin/sh

set -e

B=$(readlink "$0" || echo "$0")
D=$(cd ${B%/*} && pwd -P)

PROG=$0
debug() {
    echo "# DEBUG(${PROG}): $@" >& 2
}

if [ $# -lt 4 ]; then
    echo "usage: $0 ds snap-name-regex period-in-sec num-keep"
    echo "note: negative num-keep means to skip gc"
    exit 2
fi

DS=$1
REGEX=$2
PERIOD=$3
NKEEP=$4

#debug "DS ${DS}"
#debug "REGEX ${REGEX}"
#debug "PERIOD ${PERIOD}"
#debug "NKEEP ${NKEEP}"

if [ "$NKEEP" -lt 0 ]; then
    exit 0
fi
THRESH=$((NKEEP * PERIOD))
${D}/list_victims.sh ${DS} "${REGEX}" ${THRESH} ${NKEEP} | while read x; do
    snapname=${x#*@}
    test -n "${snapname}"
    debug zfs destroy -r $x
    zfs destroy -r $x
done
