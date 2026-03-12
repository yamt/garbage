#include <sys/atomic.h>
#include <sys/cpuio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <paths.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned int
getnp(void)
{
        int fd = open(_PATH_CPUCTL, O_RDWR);
        if (fd == -1) {
                fprintf(stderr, "failed to open %s, errno=%d (%s)\n",
                        _PATH_CPUCTL, errno, strerror(errno));
                exit(1);
        }
        unsigned int np;
        int ret = ioctl(fd, IOC_CPU_GETCOUNT, &np);
        if (ret == -1) {
                fprintf(stderr, "IOC_CPU_GETCOUNT failed, errno=%d (%s)\n",
                        errno, strerror(errno));
                exit(1);
        }
        close(fd);
        return np;
}

/* caller should set s->cs_id before calling us */
void
getcpustate(cpustate_t *s)
{
        int fd = open(_PATH_CPUCTL, O_RDWR);
        if (fd == -1) {
                fprintf(stderr, "failed to open %s, errno=%d (%s)\n",
                        _PATH_CPUCTL, errno, strerror(errno));
                exit(1);
        }
        int ret = ioctl(fd, IOC_CPU_GETSTATE, s);
        if (ret == -1) {
                fprintf(stderr, "IOC_CPU_GETSTATE failed, errno=%d (%s)\n",
                        errno, strerror(errno));
                exit(1);
        }
        close(fd);
}

uint64_t
timestamp(void)
{
        struct timespec ts;
        int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
        if (ret != 0) {
                fprintf(stderr, "clock_gettime failed, errno=%d(%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void
busyloop(unsigned int ns)
{
        uint64_t start = timestamp();
        while (timestamp() - start < ns) {
        }
}

struct cpu_info {
        unsigned int count;
};

struct thread_info {
        pthread_t t;
        unsigned int i;
        struct cpu_info *cs;
};

void *
thread_func(void *vp)
{
        struct thread_info *t = vp;
        unsigned int i;
        for (i = 0; i < 10; i++) {
                unsigned int curcpu = pthread_curcpu_np();
                printf("thread [%u]: curcpu=%d\n", t->i, curcpu);
                t->cs[curcpu].count++;
                busyloop(1000000000);
        }
        return NULL;
}

int
main(void)
{
        unsigned int i;
        unsigned int np = getnp();
        unsigned int nt = np / 2;

        unsigned int curcpu = pthread_curcpu_np();
        printf("main: curcpu=%d\n", curcpu);

        struct thread_info *ts = calloc(nt, sizeof(*ts));
        if (ts == NULL) {
                fprintf(stderr, "malloc failed\n");
                exit(1);
        }
        for (i = 0; i < nt; i++) {
                struct thread_info *t = &ts[i];
                t->cs = calloc(np, sizeof(*t->cs));
                if (t->cs == NULL) {
                        fprintf(stderr, "malloc failed\n");
                        exit(1);
                }
        }

        for (i = 0; i < nt; i++) {
                struct thread_info *t = &ts[i];
                int ret;
                t->i = i;
                ret = pthread_create(&t->t, NULL, thread_func, t);
                if (ret != 0) {
                        fprintf(stderr,
                                "pthread_create failed, error=%d(%s)\n", ret,
                                strerror(ret));
                        exit(1);
                }
        }
        for (i = 0; i < nt; i++) {
                struct thread_info *t = &ts[i];
                void *value;
                int ret = pthread_join(t->t, &value);
                if (ret != 0) {
                        fprintf(stderr, "pthread_join failed, error=%d(%s)\n",
                                ret, strerror(ret));
                        exit(1);
                }
        }
        for (i = 0; i < np; i++) {
                cpustate_t s;
                s.cs_id = i;
                getcpustate(&s);
                printf("cpu [%3u] [hwid %3" PRIu32 "]", i, s.cs_hwid);
                unsigned int j;
                for (j = 0; j < nt; j++) {
                        printf(" %3u", ts[j].cs[i].count);
                }
                printf("\n");
        }
}
