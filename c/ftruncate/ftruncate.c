#include <sys/fcntl.h>

#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
        int ret;
        if (argc != 3) {
                fprintf(stderr, "arg\n");
                exit(2);
        }
        off_t size;
        const char *filename = argv[1];
        const char *cp = argv[2];
        char *ep;
        errno = 0;
        uintmax_t x = strtoumax(cp, &ep, 0);
        if (cp == ep || *ep != 0 || errno != 0) {
                fprintf(stderr, "invalid filesize %s\n", cp);
                exit(1);
        }
        size = x;
        int fd = open(filename, O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
                fprintf(stderr, "failed to open %s, errno=%d (%s)\n", filename,
                        errno, strerror(errno));
                exit(1);
        }
        ret = ftruncate(fd, size);
        if (ret != 0) {
                fprintf(stderr, "ftruncate failed, errno=%d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
}
