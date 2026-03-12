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
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
                fprintf(stderr, "open '%s' failed, error=%d (%s)\n", filename,
                        errno, strerror(errno));
                exit(1);
        }

        while (1) {
                char path[PATH_MAX];
                int ret;

                ret = fcntl(fd, F_GETPATH, path);
                if (ret == -1) {
                        fprintf(stderr,
                                "fcntl F_GETPATH failed, error=%d (%s)\n",
                                errno, strerror(errno));
                        exit(1);
                }
                printf("F_GETPATH: '%s'\n", path);
                sleep(5);
        }
}
