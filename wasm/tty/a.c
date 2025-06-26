#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(__wasi__)
#include <wasi/api.h>

#define DEFINE_BIT(a, b) [b] = #a,
const char *right_names[] = {
        /* clang-format off */
DEFINE_BIT(WASI_RIGHT_FD_DATASYNC, 0)
DEFINE_BIT(WASI_RIGHT_FD_READ, 1)
DEFINE_BIT(WASI_RIGHT_FD_SEEK, 2)
DEFINE_BIT(WASI_RIGHT_FD_FDSTAT_SET_FLAGS, 3)
DEFINE_BIT(WASI_RIGHT_FD_SYNC, 4)
DEFINE_BIT(WASI_RIGHT_FD_TELL, 5)
DEFINE_BIT(WASI_RIGHT_FD_WRITE, 6)
DEFINE_BIT(WASI_RIGHT_FD_ADVISE, 7)
DEFINE_BIT(WASI_RIGHT_FD_ALLOCATE, 8)
DEFINE_BIT(WASI_RIGHT_PATH_CREATE_DIRECTORY, 9)
DEFINE_BIT(WASI_RIGHT_PATH_CREATE_FILE, 10)
DEFINE_BIT(WASI_RIGHT_PATH_LINK_SOURCE, 11)
DEFINE_BIT(WASI_RIGHT_PATH_LINK_TARGET, 12)
DEFINE_BIT(WASI_RIGHT_PATH_OPEN, 13)
DEFINE_BIT(WASI_RIGHT_FD_READDIR, 14)
DEFINE_BIT(WASI_RIGHT_PATH_READLINK, 15)
DEFINE_BIT(WASI_RIGHT_PATH_RENAME_SOURCE, 16)
DEFINE_BIT(WASI_RIGHT_PATH_RENAME_TARGET, 17)
DEFINE_BIT(WASI_RIGHT_PATH_FILESTAT_GET, 18)
DEFINE_BIT(WASI_RIGHT_PATH_FILESTAT_SET_SIZE, 19)
DEFINE_BIT(WASI_RIGHT_PATH_FILESTAT_SET_TIMES, 20)
DEFINE_BIT(WASI_RIGHT_FD_FILESTAT_GET, 21)
DEFINE_BIT(WASI_RIGHT_FD_FILESTAT_SET_SIZE, 22)
DEFINE_BIT(WASI_RIGHT_FD_FILESTAT_SET_TIMES, 23)
DEFINE_BIT(WASI_RIGHT_PATH_SYMLINK, 24)
DEFINE_BIT(WASI_RIGHT_PATH_REMOVE_DIRECTORY, 25)
DEFINE_BIT(WASI_RIGHT_PATH_UNLINK_FILE, 26)
DEFINE_BIT(WASI_RIGHT_POLL_FD_READWRITE, 27)
DEFINE_BIT(WASI_RIGHT_SOCK_SHUTDOWN, 28)
DEFINE_BIT(WASI_RIGHT_SOCK_ACCEPT, 29)
        /* clang-format on */
};

void
print_right_bits(uint64_t bits)
{
        size_t i;
        for (i = 0; i < sizeof(right_names) / sizeof(right_names[0]); i++) {
                if ((bits & (UINT64_C(1) << i)) == 0) {
                        continue;
                }
                printf("\t%s\n", right_names[i]);
        }
}
#endif

bool
print_fd(int fd, const char *name)
{
        bool unexpected = false;
        if (isatty(fd)) {
                printf("%s is a tty\n", name);
        } else {
                printf("%s is NOT a tty\n", name);
                unexpected = true;
        }
        int flags = fcntl(fd, F_GETFL);
        switch (flags & O_ACCMODE) {
        case O_RDONLY:
                printf("%s is O_RDONLY\n", name);
                break;
        case O_WRONLY:
                printf("%s is O_WRONLY\n", name);
                break;
        case O_RDWR:
                printf("%s is O_RDWR\n", name);
                break;
        }
#if defined(__wasi__)
        __wasi_fdstat_t sb;
        int r = __wasi_fd_fdstat_get(fd, &sb);
        if (r != 0) {
                printf("__wasi_fd_fdstat_get on %s failed with %d\n", name, r);
                unexpected = true;
        } else {
                printf("%s fs_filetype %02x\n", name, (int)sb.fs_filetype);
                printf("%s fs_rights_base %016" PRIx64 "\n", name,
                       sb.fs_rights_base);
                print_right_bits(sb.fs_rights_base);
                printf("%s fs_rights_inheriting %016" PRIx64 "\n", name,
                       sb.fs_rights_inheriting);
        }
#endif
        return unexpected;
}

int
main(void)
{
        bool unexpected = false;
        unexpected |= print_fd(STDIN_FILENO, "STDIN_FILENO");
        unexpected |= print_fd(STDOUT_FILENO, "STDOUT_FILENO");
        unexpected |= print_fd(STDERR_FILENO, "STDERR_FILENO");
        exit(unexpected);
}
