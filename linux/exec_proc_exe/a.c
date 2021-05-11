#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

const char *path = "/proc/self/exe";

int
main(int argc, char *argv[])
{
    char buf[PATH_MAX];
    int ret;

    ret = readlink(path, buf, sizeof(buf));
    if (ret == -1) {
        printf("readlink failed with %d, errno=%d\n", ret, errno);
    } else {
        printf("link contents: %.*s\n", ret, buf);
    }

    printf("argv[0] = %s\n", argv[0]);
    if (!strcmp(argv[0], "hoge")) {
        exit(0);
    }
    argv[0] = "hoge";
    ret = execve(path, argv, environ);
    printf("execve returned with %d, errno=%d\n", ret, errno);
    exit(1);
}
