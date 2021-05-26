#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

const char *proc_self_exe = "/proc/self/exe";

int
main(int argc, char *argv[])
{
    char buf[PATH_MAX];
    int ret;

    printf("argv[0] = %s\n", argv[0]);

    ret = readlink(proc_self_exe, buf, sizeof(buf));
    if (ret == -1) {
        printf("readlink %s failed with %d, errno=%d\n",
           proc_self_exe, ret, errno);
        exit(1);
    } else {
        printf("readlink %s contents: %.*s\n", proc_self_exe, ret, buf);
    }

    if (!strcmp(argv[0], "step3")) {
        exit(0);
    }

    const char *path;

    if (strcmp(argv[0], "step2")) {
        path = argv[0];
        argv[0] = "step2";
    } else {
        const char *tmp = "/tmp";
        ret = chdir(tmp); /* try to confuse qemu */
        if (ret == -1) {
            printf("chdir %s failed with %d, errno=%d\n", tmp, ret, errno);
            exit(1);
        }
        path = proc_self_exe;
        argv[0] = "step3";
    }
    if (fflush(NULL) != 0) {
        fprintf(stderr, "fflush failed\n");
        exit(1);
    }
    ret = execve(path, argv, environ);
    printf("execve %s returned with %d, errno=%d\n", path, ret, errno);
    exit(1);
}
