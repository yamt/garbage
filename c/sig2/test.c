/*
 * note: nuttx doesn't have sigsetjmp/siglongjmp
 *
 * note: in posix, it's unspecified if setjmp/longjmp save and restore
 * the signal mask. on netbsd they do. no nuttx they don't.
 *
 * note: signal handling on nuttx is a bit broken
 * https://github.com/apache/nuttx/issues/10326
 */

#include <sys/wait.h>

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int g_signo = SIGUSR1;
sigset_t g_mask;

struct env {
        pthread_t thread;
        _Atomic bool signalled;
        _Atomic bool blocking;
};

static void
assert_sigmask(bool blocked)
{
        sigset_t curmask;
        sigprocmask(SIG_BLOCK, NULL, &curmask);
        assert(blocked == sigismember(&curmask, g_signo));
}

static void
block_sig()
{
        assert_sigmask(false);
        sigprocmask(SIG_BLOCK, &g_mask, NULL);
        assert_sigmask(true);
}

static void
unblock_sig()
{
        assert_sigmask(true);
        sigprocmask(SIG_UNBLOCK, &g_mask, NULL);
        assert_sigmask(false);
}

void
sighandler(int signo)
{
        assert_sigmask(true);
        printf("%s called\n", __func__);
}

/* ---- */

int
begin_blocking_op(struct env *env)
{
        assert_sigmask(true);
        env->blocking = true;
        if (env->signalled) {
                env->blocking = false;
                return EINTR;
        }
        unblock_sig();
        return 0;
}

void
end_blocking_op(struct env *env)
{
        assert(env->blocking);
        env->blocking = false;
        block_sig();
}

void
interrupt_blocking_op(struct env *env)
{
        env->signalled = true;
        /*
         * env->blocking == true here means that the target thread
         * is in somewhere between begin_blocking_op and end_blocking_op.
         * keep waking it up until it reaches end_blocking_op, which
         * clears env->blocking.
         *
         * this dumb loop is necessary because posix doesn't provide
         * a way to unmask signal and block atomically.
         */
        while (env->blocking) {
                pthread_kill(env->thread, g_signo);

                /* relax */
                struct timespec ts;
                ts.tv_sec = 0;
                ts.tv_nsec = 50 * 1000 * 1000; /* 50ms */
                nanosleep(&ts, NULL);
        }
}

/* ---- */

ssize_t
cancellable_read(struct env *env, int fd, void *buf, size_t buflen)
{
        ssize_t ret;

        void *some_resource = malloc(1);

        ret = begin_blocking_op(env);
        if (ret != 0) {
                goto fail;
        }

        /*
         * CAVEAT: if we get the signal here, we won't notice.
         */
        printf("%s: calling read\n", __func__);
        /*
         * Note: if a blocking syscall (read below) is interrupted,
         * it causes EINTR or partial success.
         */
        errno = 0;
        ret = read(fd, buf, buflen);
        int error = errno;
        printf("%s: returned from read\n", __func__);
        printf("%s: read returned %d (%s)\n", __func__, (int)ret,
               strerror(error));
        /*
         * Note: if we get the signal here, we won't notice. it's ok
         * as we are not going to block anymore.
         */
        end_blocking_op(env);
fail:
        free(some_resource);
        return ret;
}

void *
child(void *vp)
{
        struct env *env = vp;
        int i;
        for (i = 0; i < 3; i++) {
                sleep(1);
                printf("child: sinding a signal (%s) to parent (%d/%d)\n",
                       strsignal(g_signo), i + 1, 3);
                interrupt_blocking_op(env);
        }
        return NULL;
}

int
main(int argc, char **argv)
{
        struct env env0;
        struct env *env = &env0;
        int ret;

        sigemptyset(&g_mask);
        sigaddset(&g_mask, g_signo);
        block_sig();

        env->thread = pthread_self();
        env->signalled = false;
        env->blocking = false;

        pthread_t t;
        ret = pthread_create(&t, NULL, child, env);
        assert(ret == 0);

        sleep(1);

        printf("parent\n");

        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = sighandler;
        sigaction(g_signo, &sa, NULL);

        char buf[1];
        int i;
        for (i = 0; i < 3; i++) {
                printf("parent: calling cancellable_read (%d/%d)\n", i + 1, 3);
                ret = cancellable_read(env, STDIN_FILENO, buf, sizeof(buf));
                printf("cancellable_read returned %d\n", (int)ret);
        }

        printf("waiting for the child exitting.\n");
        void *value;
        ret = pthread_join(t, &value);
        assert(ret == 0);
        printf("success\n");
        exit(0);
}
