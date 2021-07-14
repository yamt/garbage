#! /bin/sh

set -e
set -x

REF=${1:-alpine}
TARFILE=${2:-${REF}.tar}

IMG_ID=$(podman image pull $REF)
MNT=$(podman image mount $IMG_ID)
# REVISIT: what to do with metadata like file owner/group?
tar -c -f ${TARFILE} -C ${MNT} .
podman image umount $IMG_ID
