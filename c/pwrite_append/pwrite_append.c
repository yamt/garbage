#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(void)
{
        int fd = open("test.txt", O_CREAT | O_TRUNC | O_WRONLY | O_APPEND,
                      0666);
        if (fd == -1) {
                fprintf(stderr, "open failed with %d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }

        char buf[8] = "abcdefgh";
        ssize_t ssz;

        ssz = write(fd, buf, 8);
        if (ssz == -1) {
                fprintf(stderr, "write failed with %d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }

        ssz = pwrite(fd, buf, 8, 0);
        if (ssz == -1) {
                fprintf(stderr, "pwrite failed with %d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }

        off_t off = lseek(fd, 0, SEEK_END);
        if (off == 16) {
                printf("O_APPEND affected pwrite. (bsd, linux)\n");
        } else if (off == 8) {
                printf("O_APPEND didn't affect pwrite. (posix, darwin)\n");
        } else {
                printf("unknown behavior. off == %jd\n", (intmax_t)off);
        }
}
