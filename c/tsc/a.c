#include <sys/cpuio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <assert.h>
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

uint64_t
rdtsc(void)
{
        uint32_t hi, lo;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        uint64_t tsc = ((uint64_t)hi << 32) | lo;
        return tsc;
}

void
set_affinity(pthread_t self, cpuset_t *cset)
{
        int error = pthread_setaffinity_np(self, cpuset_size(cset), cset);
        if (error != 0) {
                fprintf(stderr, "pthread_setaffinity_np failed with %d\n",
                        error);
                exit(1);
        }
}

void
measure(pthread_t self, unsigned int np, cpuset_t **csets, int64_t *skews)
{
        unsigned int i;

        /*
         * measure how much tsc of the cpu advances from the next cpu.
         * + the OS overhead (context switches etc)
         */
        for (i = 0; i < np; i++) {
                uint64_t tsc;
                set_affinity(self, csets[(i + 1) % np]);
                tsc = rdtsc();
                set_affinity(self, csets[i]);
                skews[i] = rdtsc() - tsc;
        }
}

int
main(void)
{
        pthread_t self = pthread_self();
        unsigned int i;
        unsigned int np = getnp();
        int error;
        int64_t *skews = malloc(np * sizeof(*skews));
        cpuset_t **csets = malloc(np * sizeof(*csets));
        if (skews == NULL || csets == NULL) {
                exit(1);
        }
        for (i = 0; i < np; i++) {
                cpuset_t *cset = cpuset_create();
                if (cset == NULL) {
                        exit(1);
                }
                cpuset_zero(cset);
                cpuset_set(i, cset);
                csets[i] = cset;
        }

        /* ignore the first run, which hopefully makes the cache hot */
        measure(self, np, csets, skews);
        measure(self, np, csets, skews);

        for (i = 0; i < np; i++) {
                cpuset_destroy(csets[i]);
        }

        int64_t sum = 0;
        for (i = 0; i < np; i++) {
                sum += skews[i];
        }
        int64_t avg = sum / np;
        // printf("avg %" PRId64 "\n", avg);

        /*
         * assuming the OS overhead in the above measurement was a constant,
         * subtract the average, which hopefully represents the OS overhead.
         */
        for (i = 0; i < np; i++) {
                skews[i] -= avg;
        }

        /*
         * calculate skew from the cpu 0
         */
        for (i = 0; i < np; i++) {
                // printf("cpu %u skew raw %" PRId64 "\n", i, skews[i]);
                int64_t s = 0;
                unsigned int j;
                for (j = 0; j < i; j++) {
                        s -= skews[j];
                }
                printf("cpu %u skew %" PRId64 "\n", i, s);
                if (i == np-1) {
                        /* up to np errors because of integer division */
                        assert(s <= skews[i]);
                        assert(skews[i] < s + np);
                }
        }
}
