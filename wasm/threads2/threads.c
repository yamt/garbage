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
         * macOS, NetBSD:
         * close() blocks until read() finishes.
         *
         * linux:
         * close() finishes successfully. read() continues to work.
         */
        int ret = close(STDIN_FILENO);
        printf("%s: closed fd 0 with %d\n", __func__, ret);
        return NULL;
}

void *
poller(void *vp)
{
        struct pollfd pfd;
        pfd.fd = 0;
        pfd.events = POLLIN;
        printf("%s: polling on fd 0\n", __func__);
        int ret = poll(&pfd, 1, -1);
        if (ret == -1) {
                printf("%s: poll returned %d errno %d (%s)\n", __func__, ret,
                       errno, strerror(errno));
        } else {
                printf("%s: poll returned %d\n", __func__, ret);
        }
        return NULL;
}

int
main(int argc, char **argv)
{
        int ret;
        setvbuf(stdout, NULL, _IONBF, 0);

        pthread_t t[2];
        ret = pthread_create(&t[0], NULL, closer, NULL);
        if (ret != 0) {
                printf("pthread_create failed with %d\n", ret);
                exit(1);
        }
        ret = pthread_create(&t[1], NULL, poller, NULL);
        if (ret != 0) {
                printf("pthread_create failed with %d\n", ret);
                exit(1);
        }

        char buf[1];
        *buf = 0;
        printf("reading from fd 0\n");
        ssize_t ssz = read(STDIN_FILENO, buf, sizeof(buf));
        printf("read from fd 0, result '%c' ssz=%zd\n", *buf, ssz);

        printf("joining\n");
        int i;
        for (i = 0; i < 2; i++) {
                void *v;
                ret = pthread_join(t[i], &v);
                assert(ret == 0);
        }
        printf("exiting\n");
        exit(0);
}
