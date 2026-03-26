#include <sys/mount.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
        if (argc != 2) {
                fprintf(stderr, "arg\n");
                exit(2);
        }
        const char *filename = argv[1];

        size_t sz;
        int ret;
        sz = 0;
        ret = getfh(filename, NULL, &sz);
        if (ret == 0) {
                fprintf(stderr, "getfh '%s' unexpected success\n", filename);
                exit(1);
        }
        if (ret != 0 && errno != E2BIG) {
                fprintf(stderr,
                        "getfh '%s' for size probe failed with %d (%s)\n",
                        filename, errno, strerror(errno));
                exit(1);
        }
        uint8_t *fh = malloc(sz);
        if (fh == NULL) {
                fprintf(stderr, "malloc %zu failed with %d (%s)\n", sz, errno,
                        strerror(errno));
                exit(1);
        }
        ret = getfh(filename, fh, &sz);
        if (ret != 0) {
                fprintf(stderr, "getfh '%s' failed with %d (%s)\n", filename,
                        errno, strerror(errno));
                exit(1);
        }

        size_t i;
        for (i = 0; i < sz; i++) {
                printf("%02x", fh[i]);
        }
        printf("\n");

        const struct netbsd_fh {
                struct {
                        int32_t val[2];
                } fsid;
                struct {
                        unsigned short len;
                        unsigned short pad;
                        char data[];
                } fid;
        } *nfh = (const void *)fh;
        printf("fsid {0x%08" PRIx32 ", 0x%08" PRIx32 "}\n", nfh->fsid.val[0],
               nfh->fsid.val[1]);

        /*
         * assume zfs from here
         */

        /*
         * zfs on netbsd stores its fstype in the lowest 8-bits.
         * on solaris, it seems to help make fsid unique across file systems.
         * on netbsd, it doesn't make sense at all.
         * 75 == makefstype("zfs") == (('z'<<4) ^ ('f'<<2) ^ 's') & 0xff
         */
        printf("zfs fstype %u (always 75)\n", (uint8_t)nfh->fsid.val[1]);

        /*
         * this guid is 56-bit fsid_guid, which you can query with:
         *
         *   zfs get objsetid y/src
         *   zdb -dddd y 60
         *
         * this is NOT guid property you can get with "zfs get guid".
         */
        printf("zfs guid %" PRIu64 "\n",
               ((uint64_t)((uint32_t)nfh->fsid.val[1] >> 8) << 32) |
                       (uint32_t)nfh->fsid.val[0]);
}
