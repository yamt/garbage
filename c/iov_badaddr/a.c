#include <sys/types.h>
#include <sys/uio.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main()
{
        int fds[2];
        int ret;
        ret = pipe(fds);
        if (ret != 0) {
                printf("pipe error %d %s\n", errno, strerror(errno));
                exit(1);
        }
        ssize_t ssz;
        ssz = write(fds[1], "X", 1);
        if (ssz == -1) {
                printf("write error %d %s\n", errno, strerror(errno));
                exit(1);
        }
        char buf[1];
        struct iovec iov[] = {
                {
                        .iov_base = buf,
                        .iov_len = sizeof(buf),
                },
                {
                        .iov_base = (void *)1, /* bad address */
                        .iov_len = 1,
                },
        };
        ssz = readv(fds[0], iov, 2);
        if (ssz == -1) {
                printf("readv error %d %s\n", errno, strerror(errno));
                exit(1);
        }
        if (ssz != 1) {
                printf("readv unexpected size\n");
                exit(1);
        }
        if (*buf != 'X') {
                printf("readv unexpected data\n");
                exit(1);
        }
        printf("success\n");
        exit(0);
}
