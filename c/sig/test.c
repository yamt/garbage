
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

jmp_buf jb;
sigset_t mask;

void
sighandler(int signo)
{
        siglongjmp(jb, 1);
}

void
block()
{
        sigprocmask(SIG_BLOCK, &mask, NULL);
}

void
unblock()
{
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

ssize_t
cancellable_read(int fd, void *buf, size_t buflen)
{
        void *some_resource = malloc(1);
        if (sigsetjmp(jb, 1) != 0) {
                printf("%s: cancelled\n", __func__);
                free(some_resource);
                return EINTR;
        }
        unblock();
        ssize_t ret = read(fd, buf, buflen);
        printf("%s: read returned %d\n", __func__, (int)ret);
        /*
         * CAVEAT: if we get the signal here, we loose the result of
         * the above system call.
         * depending on applications, it might or might not be a problem.
         */
        block();
        free(some_resource);
        return ret;
}

int
main(int argc, char **argv)
{
        int signo = SIGINT; /* use SIGUSRx instead for real apps */
        sigemptyset(&mask);
        sigaddset(&mask, signo);

        block();
        signal(signo, sighandler);

        char buf[1];
        ssize_t ret = cancellable_read(STDIN_FILENO, buf, sizeof(buf));
        printf("cancellable_read returned %d\n", (int)ret);

        exit(0);
}
