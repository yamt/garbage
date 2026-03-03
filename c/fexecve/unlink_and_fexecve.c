#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
        int ret;
        if (argc < 2) {
                fprintf(stderr, "args\n");
                exit(2);
        }
        const char *file = argv[1];
        int fd = open(file, O_EXEC);
        if (fd == -1) {
                fprintf(stderr, "failed to open '%s', errno=%d (%s)\n", file,
                        errno, strerror(errno));
                exit(1);
        }
        /* VOP_REMOVE drops the name cache entry even if it fails */
        ret = unlink(file);
        if (ret == -1) {
                fprintf(stderr, "ignoring unlink failrue '%s', errno=%d (%s)\n", file,
                        errno, strerror(errno));
        }
        int cargc = argc - 1;
        char **cargv = malloc(cargc + 1);
        if (cargv == NULL) {
                fprintf(stderr, "malloc failed\n");
                exit(1);
        }
        cargv[0] = "hoge";
        unsigned int i;
        for (i = 1; i <= cargc; i++) {
                cargv[i] = argv[i + 1];
        }
        extern char **environ;
        ret = fexecve(fd, cargv, environ);
        assert(ret == -1);
        fprintf(stderr, "fexecve failed, errno=%d (%s)\n", errno,
                strerror(errno));
        exit(1);
}
