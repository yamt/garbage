#! /bin/sh

set -e

BINFMT_MISC_MOUNT=/proc/sys/fs/binfmt_misc
REGISTER=${BINFMT_MISC_MOUNT}/register
if [ ! -f ${REGISTER} ]; then
    mount binfmt_misc -t binfmt_misc ${BINFMT_MISC_MOUNT}
fi

DATA=':qemu-aarch64:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\xb7\x00:\xff\xff\xff\xff\xff\xff\xff\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff:/yamt/qemu-aarch64:FPO'

echo -1 > ${BINFMT_MISC_MOUNT}/qemu-aarch64
echo ${DATA} > ${REGISTER}

cat ${BINFMT_MISC_MOUNT}/qemu-aarch64
