#include <stdio.h>

int
main()
{
	__wasi_fdstat_t buf;
    int r = __wasi_fd_fdstat_get(0, &buf);
    printf("__wasi_fd_fdstat_get returned %d\n", r);
    printf("fs_filetype %jx\n", (uintmax_t)buf.fs_filetype);
    printf("fs_rights_base %jx\n", (uintmax_t)buf.fs_rights_base);
}
