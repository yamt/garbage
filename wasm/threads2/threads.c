/*
 * test poll/read vs close behavior
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *
closer(void *vp)
{
        printf("%s: sleeping 1 sec\n", __func__);
        sleep(1);
        printf("%s: closing fd 0\n", __func__);
        /*
         * macOS:
         * read() blocks close().
         * poll() doesn't block close().
         * close() doesn't wake up poll() or read().
         * for some reasons, after close() started, poll() doesn't
         * seem reacting on the events. (bug?)
         *
         * NetBSD:
         * read() blocks close().
         * poll() doesn't block close().
         * close() doesn't wake up poll() or read().
         * on the next wake up, poll() returns POLLNVAL.
         *
         * linux:
         * read() doesn't block close().
         * poll() doesn't block close().
         * close() doesn't wake up poll() or read().
         * on the next wake up, poll() returns POLLNVAL.
         */
        int ret = close(STDIN_FILENO);
        printf("%s: closed fd 0 with %d\n", __func__, ret);
        printf("%s: exiting\n", __func__);
        return NULL;
}

void *
poller(void *vp)
{
        struct pollfd pfd0;
        struct pollfd *pfd = &pfd0;
        pfd[0].fd = STDIN_FILENO;
        pfd[0].events = POLLIN;
        printf("%s: POLLIN=%02x POLLHUP=%02x POLLERR=%02x POLLNVAL=%0x\n",
               __func__, POLLIN, POLLHUP, POLLERR, POLLNVAL);
        printf("%s: polling on fd 0\n", __func__);
        int ret = poll(pfd, 1, -1);
        if (ret == -1) {
                printf("%s: poll returned %d errno %d (%s)\n", __func__, ret,
                       errno, strerror(errno));
        } else {
                printf("%s: poll returned %d\n", __func__, ret);
                if (ret >= 0) {
                        int i;
                        for (i = 0; i < ret; i++) {
                                printf("%s: pfd[%d] revent %04x\n", __func__,
                                       i, pfd[i].revents);
                        }
                }
        }
        printf("%s: exiting\n", __func__);
        return NULL;
}

int
main(int argc, char **argv)
{
        int ret;
        setvbuf(stdout, NULL, _IONBF, 0);

        pthread_t t[4];
        pthread_t *pt = t;
        int i;
        for (i = 0; i < 3; i++) {
                ret = pthread_create(pt, NULL, closer, NULL);
                if (ret != 0) {
                        printf("pthread_create failed with %d\n", ret);
                        exit(1);
                }
                pt++;
        }

        ret = pthread_create(pt, NULL, poller, NULL);
        if (ret != 0) {
                printf("pthread_create failed with %d\n", ret);
                exit(1);
        }
        pt++;

        char buf[1];
        *buf = 0;
        printf("reading from fd 0\n");
        ssize_t ssz = read(STDIN_FILENO, buf, sizeof(buf));
        printf("read from fd 0, result '%c' ssz=%zd\n", *buf, ssz);

        printf("joining\n");
        for (i = 0; i < pt - t; i++) {
                printf("joining [%d]\n", i);
                void *v;
                ret = pthread_join(t[i], &v);
                assert(ret == 0);
        }
        printf("exiting\n");
        exit(0);
}
