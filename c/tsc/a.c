#include <sys/cpuio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <inttypes.h>
#include <paths.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned int
getnp(void)
{
        int fd = open(_PATH_CPUCTL, O_RDWR);
        if (fd == -1) {
                fprintf(stderr, "failed to open %s, errno=%d\n", _PATH_CPUCTL,
                        errno);
                exit(1);
        }
        unsigned int np;
        int ret = ioctl(fd, IOC_CPU_GETCOUNT, &np);
        if (ret == -1) {
                fprintf(stderr, "IOC_CPU_GETCOUNT failed, errno=%d\n",
                        _PATH_CPUCTL, errno);
                exit(1);
        }
        close(fd);
        return np;
}

void
print_tsc(void)
{
        uint32_t hi, lo;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        uint32_t cpu = pthread_curcpu_np();
        uint64_t tsc = ((uint64_t)hi << 32) | lo;
        printf("cpu %u tsc %" PRIu64 "\n", cpu, tsc);
}

int
main(void)
{
        cpuset_t *cset = cpuset_create();
        pthread_t self = pthread_self();
        unsigned int i;
        unsigned int np = getnp();
        int error;

        for (i = 0; i < np; i++) {
                /* set affinity */
                cpuset_zero(cset);
                cpuset_set(i, cset);
                error = pthread_setaffinity_np(self, cpuset_size(cset), cset);
                if (error != 0) {
                        fprintf(stderr,
                                "pthread_setaffinity_np failed with %d\n",
                                error);
                        exit(1);
                }

                print_tsc();
        }
        cpuset_destroy(cset);
}
