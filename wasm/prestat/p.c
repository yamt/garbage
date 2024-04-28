#include <limits.h>
#include <stdint.h>
#include <stdio.h>

struct prestat {
        uint8_t tag;
        size_t len;
};

int prestat_get(int fd, struct prestat *prestat)
        __attribute__((import_module("wasi_unstable")))
        __attribute__((import_name("fd_prestat_get")));

int prestat_dir_name(int fd, char *buf, size_t buflen)
        __attribute__((import_module("wasi_unstable")))
        __attribute__((import_name("fd_prestat_dir_name")));

int
main()
{
        struct prestat prestat;
        int ret;
        int i;
        for (i = 0; i < 16; i++) {
                ret = prestat_get(i, &prestat);
                printf("prestat_get [%d] -> %d %d %zu\n", i, ret, prestat.tag,
                       prestat.len);
                if (ret != 0) {
                        continue;
                }
                char buf[PATH_MAX];
                ret = prestat_dir_name(i, buf, sizeof(buf));
                printf("prestat_dir_name [%d] -> %d %s\n", i, ret, buf);
        }
}
