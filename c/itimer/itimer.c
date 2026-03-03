#include <sys/time.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
sighandler(int sig)
{
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
        while (1) {
                sleep(10);
                printf("hoge\n");
        }
}
