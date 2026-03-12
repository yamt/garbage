#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

sig_atomic_t got_sig;
sig_atomic_t overruns;
sig_atomic_t intervals;

void
sighandler(int sig, siginfo_t *info, void *ctx)
{
        assert(sig == SIGALRM);
        assert(info->si_signo == SIGALRM);
        assert(info->si_code == SI_TIMER);
        if (info->si_errno != 0) {
                fprintf(stderr, "si_errno=%d (%s)\n", info->si_errno,
                        strerror(info->si_errno));
                exit(1);
        }
        got_sig++;
        timer_t timer = *(const timer_t *)info->si_value.sival_ptr;
        int ret = timer_getoverrun(timer);
        if (ret == -1) {
                fprintf(stderr,
                        "timer_getoverrun failed, errno=%d "
                        "(%s)\n",
                        errno, strerror(errno));
                exit(1);
        }
        overruns += ret;
        intervals += ret + 1;
}

int
main(int argc, char **argv)
{
        struct itimerspec it;
        struct itimerspec oit;
        int ret;

        struct sigaction sa;
        sa.sa_sigaction = sighandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO;
        ret = sigaction(SIGALRM, &sa, NULL);
        if (ret != 0) {
                fprintf(stderr, "sigaction failed, errno=%d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }

        clockid_t id = CLOCK_REALTIME;
        timer_t timer;
        struct sigevent ev;
        ev.sigev_notify = SIGEV_SIGNAL;
        ev.sigev_signo = SIGALRM;
        ev.sigev_value.sival_ptr = &timer;
        ret = timer_create(id, &ev, &timer);
        if (ret != 0) {
                fprintf(stderr, "timer_create failed, errno=%d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }
        uint64_t interval_ns = 1000000000;
        it.it_interval.tv_sec = interval_ns / 1000000000;
        it.it_interval.tv_nsec = interval_ns % 1000000000;
        it.it_value.tv_sec = interval_ns / 1000000000;
        it.it_value.tv_nsec = interval_ns % 1000000000;
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
                        printf("%u: %12" PRIu64 "ns frm start, %10" PRIu64
                               "ns frm prev, ov=%d, int=%d\n",
                               n, now_ns - start_ns, now_ns - prev_ns,
                               (int)overruns, (int)intervals);
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
