/*
 * note: nuttx doesn't have siglongjmp/sigsetjmp
 *
 * note: signal handling on nuttx is a bit broken
 * https://github.com/apache/nuttx/issues/10326
 */

#include <sys/wait.h>

#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

jmp_buf jb;
sigset_t mask;

void
sighandler(int signo)
{
        longjmp(jb, 1);
}

static void
block()
{
        sigprocmask(SIG_BLOCK, &mask, NULL);
}

static void
unblock()
{
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

ssize_t
cancellable_read(int fd, void *buf, size_t buflen)
{
        void *some_resource = malloc(1);
        if (setjmp(jb) != 0) {
                printf("%s: cancelled\n", __func__);
                free(some_resource);
                return EINTR;
        }
        unblock();
        printf("%s: calling read\n", __func__);
        ssize_t ret = read(fd, buf, buflen);
        printf("%s: returned from read\n", __func__);
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
        int signo = SIGUSR1;

        sigemptyset(&mask);
        sigaddset(&mask, signo);
        block();

        pid_t pid = fork();
        if (pid == -1) {
                printf("fork failed\n");
                exit(1);
        }
        if (pid == 0) {
                /* child */
                pid_t parent = getppid();
                int i;
                for (i = 0; i < 3; i++) {
                        sleep(1);
                        printf("child: sinding a signal (%s) to %u (%d/%d)\n",
                               strsignal(signo), (int)parent, i + 1, 3);
                        kill(parent, signo);
                }
                _Exit(0);
        }

        printf("parent: pid %u child %u\n", (int)getpid(), pid);
        signal(signo, sighandler);

        char buf[1];
        int i;
        for (i = 0; i < 3; i++) {
                printf("parent: calling cancellable_read (%d/%d)\n", i + 1, 3);
                int ret = cancellable_read(STDIN_FILENO, buf, sizeof(buf));
                printf("cancellable_read returned %d\n", (int)ret);
        }

        printf("waiting for the child %u exit.\n", (int)pid);
        int status;
        do {
                pid_t waited = waitpid(pid, &status, 0);
                if (waited == -1) {
                        printf("waitpid failed\n");
                        exit(1);
                }
        } while (!WIFEXITED(status));
        printf("success\n");
        exit(0);
}
