#include <sys/time.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

sig_atomic_t got_sig;

void
sighandler(int sig)
{
        assert(sig == SIGALRM);
        got_sig++;
}

int
main(int argc, char **argv)
{
        struct itimerval it;
        struct itimerval oit;
        int ret;

        signal(SIGALRM, sighandler);

        it.it_interval.tv_sec = 1;
        it.it_interval.tv_usec = 0;
        it.it_value.tv_sec = 1;
        it.it_value.tv_usec = 0;
        ret = setitimer(ITIMER_REAL, &it, &oit);
        if (ret != 0) {
                fprintf(stderr, "setitimer failed, errno=%d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        clockid_t id = CLOCK_REALTIME;
        uint64_t start_ns;
        uint64_t prev_ns = 0;
        unsigned int n = 0;
        while (1) {
                struct timespec now;
                ret = clock_gettime(id, &now);
                if (ret != 0) {
                        fprintf(stderr,
                                "clock_gettime failed, errno=%d (%s)\n", errno,
                                strerror(errno));
                        exit(1);
                }
                uint64_t now_ns = now.tv_sec * 1000000000 + now.tv_nsec;
                if (n == 0) {
                        start_ns = now_ns;
                } else {

                        printf("%u th wakeup: %" PRIu64
                               " ns from the start. %" PRIu64
                               " ns from the previous.\n",
                               n, now_ns - start_ns, now_ns - prev_ns);
                }
                prev_ns = now_ns;
                n++;

                struct timespec t;
                t.tv_sec = 10;
                t.tv_nsec = 0;
                ret = clock_nanosleep(id, 0, &t, &t);
                if (ret != 0) {
                        if (ret != EINTR) {
                                fprintf(stderr,
                                        "clock_nanosleep failed, errno=%d "
                                        "(%s)\n",
                                        ret, strerror(ret));
                                exit(1);
                        }
                        int x = got_sig;
                        assert(x);
                        if (x > 1) {
                                printf("got_sig %d\n", x);
                        }
                }
                got_sig = 0;
        }
}
