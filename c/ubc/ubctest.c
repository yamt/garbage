/*
 * A quick UBC (unified buffer cache) test program
 *
 * Note: "(should be ... with UBC)" messages means that
 * it isn't generally true for non-UBC systems.
 */

#include <sys/fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

size_t page_size; /* the unit for mmap dirtiness tracking */

ssize_t
xpwrite(int fd, const void *buf, size_t len, off_t off)
{
        size_t ssz = pwrite(fd, buf, len, off);
        if (ssz == -1) {
                fprintf(stderr, "pwrite failed: errno=%d(%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        return ssz;
}

ssize_t
xpread(int fd, void *buf, size_t len, off_t off)
{
        size_t ssz = pread(fd, buf, len, off);
        if (ssz == -1) {
                fprintf(stderr, "pread failed: errno=%d(%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        return ssz;
}

int
xmsync(void *p, size_t len, int flags)
{
        int ret = msync(p, len, flags);
        if (ret == -1) {
                fprintf(stderr, "msync failed: errno=%d(%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        return ret;
}

int
xftruncate(int fd, off_t off)
{
        int ret = ftruncate(fd, off);
        if (ret == -1) {
                fprintf(stderr, "ftruncate failed: errno=%d(%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        return ret;
}

void
set_data(uint8_t *p, uint8_t addend)
{
        p[0] = 1 + addend;
        p[page_size - 1] = 2 + addend;
        p[page_size] = 3 + addend;
        p[2 * page_size - 1] = 4 + addend;
        p[2 * page_size] = 5 + addend;
        p[3 * page_size - 1] = 6 + addend;
}

void
mmap_read_and_print(const uint8_t *p, const char *extra)
{
        if (extra != NULL) {
                printf("mmap read: %u %u %u %u %u %u (%s)\n", p[0],
                       p[page_size - 1], p[page_size], p[2 * page_size - 1],
                       p[2 * page_size], p[3 * page_size - 1], extra);
        } else {
                printf("mmap read: %u %u %u %u %u %u\n", p[0],
                       p[page_size - 1], p[page_size], p[2 * page_size - 1],
                       p[2 * page_size], p[3 * page_size - 1]);
        }
}

void
inval_nth_page(uint8_t *p, unsigned int n)
{
        if (n == 0) {
                return;
        }
        printf("invalidating %u-th page\n", n);
        uint8_t *pgaddr = &p[(n - 1) * page_size];
        xmsync(pgaddr, page_size - 1, MS_ASYNC | MS_INVALIDATE);
}

int
prepare_file(const char *filename, size_t filesize)
{
        int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
                fprintf(stderr, "failed to open %s: errno=%d(%s)\n", filename,
                        errno, strerror(errno));
                exit(1);
        }
        uint8_t *buf = malloc(filesize);
        if (buf == NULL) {
                fprintf(stderr, "malloc failed: errno=%d(%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        memset(buf, 0, filesize);
        xpwrite(fd, buf, filesize, 0);
        free(buf);
        return fd;
}

int
main(int argc, char **argv)
{
        int ret;
        if (argc != 2) {
                fprintf(stderr, "arg\n");
                exit(2);
        }
        page_size = sysconf(_SC_PAGESIZE);
        printf("using page_size=%zu\n", page_size);
        const char *filename = argv[1];
        int fd = prepare_file(filename, 3 * page_size);
        void *vp = mmap(NULL, 3 * page_size, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_FILE, fd, 0);
        if (vp == MAP_FAILED) {
                fprintf(stderr, "mmap failed: errno=%d(%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        uint8_t *p = vp;

        printf("===== mmap read vs pwrite\n");

        printf("mmaped read of the first byte %u\n", *p);
        uint8_t one = 1;
        printf("writing 1 with pwrite...\n");
        xpwrite(fd, &one, 1, 0);
        printf("mmaped read of the first byte %u (should be 1 with UBC)\n",
               *p);
        printf("msync...\n");
        xmsync(p, 1, MS_ASYNC | MS_INVALIDATE);
        printf("mmaped read of the first byte %u (after msync)\n", *p);

        printf("===== mmap write vs pread\n");

        uint8_t d;
        xpread(fd, &d, 1, 0);
        printf("pread of the first byte %u\n", d);
        printf("writing 2 with mmap...\n");
        *p = 2;
        xpread(fd, &d, 1, 0);
        printf("pread of the first byte %u (should be 2 with UBC)\n", d);
        printf("msync...\n");
        xmsync(p, 1, MS_ASYNC);
        xpread(fd, &d, 1, 0);
        printf("pread of the first byte %u (after msync)\n", d);

        printf("===== mmap write vs pwrite\n");
        set_data(p, 10);
        mmap_read_and_print(p, NULL);
        uint8_t data[3 * page_size];
        memset(data, 0, sizeof(data));
        set_data(data, 20);
        printf("pwrite...\n");
        xpwrite(fd, &data[page_size - 1], page_size + 2, page_size - 1);
        mmap_read_and_print(p, "should be 11 22 23 24 25 16 with UBC");
        printf("msync...\n");
        xmsync(p, 3 * page_size, MS_ASYNC | MS_INVALIDATE);
        mmap_read_and_print(p, NULL);

        printf("===== self read/write (a naive impl can deadlock)\n");

        set_data(p, 30);
        printf("msync...\n");
        xmsync(p, 3 * page_size, MS_ASYNC | MS_INVALIDATE);
        mmap_read_and_print(p, "initial");
        unsigned int i;
        for (i = 0; i < 4; i++) {
                off_t off = page_size / 2;
                inval_nth_page(p, i);
                printf("pwrite...\n");
                xpwrite(fd, p + off, 2 * page_size, off);
                mmap_read_and_print(p, NULL);
                inval_nth_page(p, i);
                printf("pread...\n");
                xpread(fd, p + off, 2 * page_size, off);
                mmap_read_and_print(p, NULL);
        }

        printf("===== truncate\n");

        set_data(p, 40);
        mmap_read_and_print(p, "initial");
        printf("ftruncate...\n");
        xftruncate(fd, page_size / 2);
#if 0
        /*
         * on some systems, this SEGV. on others, this reads 0 past EOF.
         * the behavior is even block-size dependant. (eg. NetBSD ffs)
         */
        mmap_read_and_print(p, NULL);
#endif
        xpwrite(fd, &one, 1, 3 * page_size - 1);
        mmap_read_and_print(p, NULL);

        close(fd);
}
