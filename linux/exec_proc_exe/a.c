#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

int
main(int argc, char *argv[])
{
    int ret;

    printf("argv[0] = %s\n", argv[0]);
    if (!strcmp(argv[0], "hoge")) {
        exit(0);
    }
    argv[0] = "hoge";
    ret = execve("/proc/self/exe", argv, environ);
    printf("execve returned with %d, errno=%d\n", ret, errno);
    exit(1);
}
