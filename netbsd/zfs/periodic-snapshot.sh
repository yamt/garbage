#! /bin/sh

# this is meant to be used in crontab(5).
# for example,
#
# 0  *  *  *  *       -n -s periodic-snapshot.sh x hourly- 60 24
# 0  3  *  *  *       -n -s periodic-snapshot.sh x daily- 1440 7
# 0  3  *  *  1       -n -s periodic-snapshot.sh x weekly- 10080 5
# 0  3  1  *  *       -n -s periodic-snapshot.sh x monthly- 43200 -1

set -e

B=$(readlink "$0" || echo "$0")
D=$(cd ${B%/*} && pwd -P)

debug() {
    echo "# DEBUG: $@"
}

if [ $# -lt 4 ]; then
    echo "usage: $0 ds prefix period-in-sec num-keep"
    echo "note: negative num-keep means to skip gc"
    exit 2
fi

DS=$1
PREFIX=$2
PERIOD=$3
NKEEP=$4

# take a new snapshot
TS=$(date +%F-%H:%M:%S-%Z)
SNAPNAME=${PREFIX}${TS}
debug zfs snap -r ${DS}@${SNAPNAME}
zfs snap -r ${DS}@${SNAPNAME}

# remove old ones
if [ "$NKEEP" -lt 0 ]; then
    exit 0
fi
THRESH=$((NKEEP * PERIOD))
${D}/list_victims.sh ${DS} "${PREFIX}.*" ${THRESH} ${NKEEP} | while read x; do
    debug zfs destroy -r $x
    zfs destroy -r $x
done
