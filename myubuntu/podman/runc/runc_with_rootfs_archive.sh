#! /bin/sh

set -e
set -x

NAME=$1
TARFILE=$2
test -f ${TARFILE}

RUNC=${RUNC:-crun}

TMPDIR=$(mktemp -d /tmp/${NAME}.XXXXXXX)

ROOTFS=${TMPDIR}/rootfs
mkdir -p ${ROOTFS}
# REVISIT: what to do with metadata like file owner/group?
tar -x -C ${ROOTFS} -f ${TARFILE}

cd $TMPDIR
# REVISIT: probably the default config.json is not one-size-fits-all.
# REVISIT: hostname
# REVISIT: network config
${RUNC} spec
# REVISIT: detached operation, tty handling
${RUNC} run ${NAME} 
