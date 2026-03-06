#include <sys/mman.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
        if (argc != 2) {
                printf("arg\n");
                exit(2);
        }
        const char *filename = argv[1];
        int fd = open(filename, O_RDWR);
        if (fd == -1) {
                printf("open failed\n");
                exit(1);
        }
        struct stat st;
        if (fstat(fd, &st) != 0) {
                printf("fstat failed\n");
                exit(1);
        }
        off_t size = st.st_size;
        void *vp = mmap(NULL, size, PROT_READ | PROT_WRITE,
                        MAP_FILE | MAP_SHARED, fd, 0);
        if (vp == MAP_FAILED) {
                printf("mmap failed\n");
                exit(1);
        }
        printf("mmaped\n");
        uint8_t *p = vp;
        while (1) {
                p[random() % size]++;
                // msync(vp, st.st_size, MS_ASYNC);
                // sleep(1);
        }
}
