#if defined(__linux__)
#include <sys/sysmacros.h>
typedef unsigned int devmajor_t;
typedef unsigned int devminor_t;
#else
#include <sys/types.h>
#endif

#include <sys/stat.h>

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
check_dev_t(dev_t odev)
{
        devmajor_t maj = major(odev);
        devminor_t min = minor(odev);

        printf("dev_t %016" PRIx64 "\n", (uint64_t)odev);
        printf("major %08" PRIx64 " minor %08" PRIx64 "\n", (uint64_t)maj,
               (uint64_t)min);
        uint64_t dev = makedev(maj, min);
        printf("makedev %016" PRIx64 "\n", dev);
        if (dev != odev) {
                fprintf(stderr,
                        "no round trip! %016" PRIx64 " != %016" PRIx64 "\n",
                        dev, odev);
                exit(1);
        }
}

int
main(int argc, char **argv)
{
        if (argc != 2) {
                printf("arg\n");
                exit(2);
        }
        const char *filename = argv[1];
        struct stat st;
        int ret;
        ret = lstat(filename, &st);
        if (ret == -1) {
                fprintf(stderr, "lstat failed, error=%d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        check_dev_t(st.st_dev);
}
