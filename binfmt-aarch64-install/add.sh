#! /bin/sh

set -e

ARCH=$1
MAGIC=$2
MASK=$3

OUR_ENTRY=qemu-${ARCH}-yamt

BINFMT_MISC_MOUNT=/proc/sys/fs/binfmt_misc
REGISTER=${BINFMT_MISC_MOUNT}/register
if [ ! -f ${REGISTER} ]; then
    mount binfmt_misc -t binfmt_misc ${BINFMT_MISC_MOUNT}
fi

echo "Listing existing entries..."
ls ${BINFMT_MISC_MOUNT}

echo "Removing possibly conflicting entries..."
# ours
ENTRIES=$OUR_ENTRY
# docker desktop
ENTRIES="$ENTRIES qemu-${ARCH}"
# multiarch/qemu-user-static
ENTRIES="$ENTRIES qemu-${ARCH}-static"
for e in $ENTRIES; do
    ENTRY=${BINFMT_MISC_MOUNT}/$e
    if [ -f ${ENTRY} ]; then
        echo "Removing $e..."
        echo -1 > ${ENTRY}
    fi
done

conv () {
	sed -e :a -e 's/\(.*[0-9a-f]\)\([0-9a-f]\{2\}\)/\1\\x\2/;ta' -e 's/\(^\)\([0-9a-f]\{2\}\)/\1\\x\2/;ta'
}
MAGIC_BIN="$(echo ${MAGIC} | conv)"
MASK_BIN="$(echo ${MASK} | conv)"

DATA=":${OUR_ENTRY}:M::${MAGIC_BIN}:${MASK_BIN}:/yamt/qemu-${ARCH}:FPO"
echo ${DATA} > ${REGISTER}

echo "Registered entry:"
cat ${BINFMT_MISC_MOUNT}/${OUR_ENTRY}
