#! /bin/sh

set -e

OUR_ENTRY=qemu-aarch64-yamt

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
ENTRIES="$ENTRIES qemu-aarch64"
# multiarch/qemu-user-static
ENTRIES="$ENTRIES qemu-aarch64-static"
for e in $ENTRIES; do
    ENTRY=${BINFMT_MISC_MOUNT}/$e
    if [ -f ${ENTRY} ]; then
        echo "Removing $e..."
        echo -1 > ${ENTRY}
    fi
done

DATA=":${OUR_ENTRY}:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\xb7\x00:\xff\xff\xff\xff\xff\xff\xff\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff:/yamt/qemu-aarch64:FPO"
echo ${DATA} > ${REGISTER}

echo "Registered entry:"
cat ${BINFMT_MISC_MOUNT}/${OUR_ENTRY}
