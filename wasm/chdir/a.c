
/*
 * note:
 *
 * - wasi-libc getcwd always starts with "/"
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
print_realpath(const char *path)
{
        char buf[PATH_MAX];
        char *resolved = realpath(path, buf);
        if (resolved) {
                printf("path %s -> realpath %s\n", path, resolved);
        } else {
                printf("path %s -> realpath FAILED with %d (%s)\n", path,
                       errno, strerror(errno));
        }
}

void
print_cwd()
{
        char *cwd;
        cwd = getcwd(NULL, 0);
        if (cwd) {
                printf("%s:%u cwd %s\n", __func__, __LINE__, cwd);
        } else {
                printf("getcwd failed with %d (%s)\n", errno, strerror(errno));
        }
}

void
do_chdir(const char *path)
{
        print_realpath(path);
        printf("chdir into %s\n", path);
        int ret = chdir(path);
        if (ret != 0) {
                printf("%s:%u chdir to %s FAILED with %d (%s)\n", __func__,
                       __LINE__, path, errno, strerror(errno));
                exit(1);
        }
}

void
cat(const char *path)
{
        printf("========= start dumping %s\n", path);
        FILE *fp = fopen(path, "r");
        if (fp == NULL) {
                printf("failed to open %s: %s\n", path, strerror(errno));
                exit(1);
        }
        while (1) {
                char buf[100];
                size_t sz = fread(buf, 1, sizeof(buf), fp);
                if (sz == 0) {
                        break;
                }
                size_t wsz = fwrite(buf, 1, sz, stdout);
                if (wsz != sz) {
                        printf("write error\n");
                        exit(1);
                }
        }
        printf("========= end dumping %s\n", path);
}

int
main()
{
        char *cwd;
        int ret;
        print_cwd();
#if 0
    do_chdir(".");
    print_cwd();
    do_chdir("d");
    print_cwd();
#endif
        do_chdir("/cur-dir");
        print_cwd();
        cat("build.sh");

        do_chdir("/");
        print_cwd();
        do_chdir("tmp/a/b/c");
        print_cwd();
        do_chdir("/tmp/a/b/c");
        print_cwd();
}
