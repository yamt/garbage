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
        struct itimerspec it;
        struct itimerspec oit;
        int ret;

        signal(SIGALRM, sighandler);

        clockid_t id = CLOCK_REALTIME;
        timer_t timer;
        struct sigevent ev;
        ev.sigev_notify = SIGEV_SIGNAL;
        ev.sigev_signo = SIGALRM;
        ev.sigev_value.sival_int = 1;
        ret = timer_create(id, &ev, &timer);
        if (ret != 0) {
                fprintf(stderr, "timer_create failed, errno=%d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        it.it_interval.tv_sec = 1;
        it.it_interval.tv_nsec = 0;
        it.it_value.tv_sec = 1;
        it.it_value.tv_nsec = 0;
        ret = timer_settime(timer, 0, &it, &oit);
        if (ret != 0) {
                fprintf(stderr, "timer_settime failed, errno=%d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
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
                        int overruns = timer_getoverrun(timer);
                        if (overruns == -1) {
                                fprintf(stderr,
                                        "timer_getoverrun failed, errno=%d "
                                        "(%s)\n",
                                        errno, strerror(errno));
                                exit(1);
                        }
                        printf("%u th wakeup: %12" PRIu64
                               "ns from start. %10" PRIu64
                               "ns from prev. overruns=%d\n",
                               n, now_ns - start_ns, now_ns - prev_ns,
                               overruns);
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
