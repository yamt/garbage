#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *
start(void *vp)
{
        printf("sleeping 1 sec\n");
        sleep(1);
        printf("closing fd 0\n");
        /*
         * macOS, NetBSD:
         * close() blocks until read() finishes.
         *
         * linux:
         * close() finishes successfully. read() continues to work.
         */
        int ret = close(STDIN_FILENO);
        printf("closed fd 0 with %d\n", ret);
        return NULL;
}

int
main(int argc, char **argv)
{
        int ret;
        setvbuf(stdout, NULL, _IONBF, 0);

        unsigned int n = 1;

        pthread_t *t;
        if (n > 0) {
                t = malloc(n * sizeof(*t));
                assert(t != NULL);
        }
        unsigned int i;
        for (i = 0; i < n; i++) {
                ret = pthread_create(&t[i], NULL, start, NULL);
                if (ret != 0) {
                        printf("pthread_create failed with %d\n", ret);
                        exit(1);
                }
        }

        char buf[1];
        *buf = 0;
        printf("reading from fd 0\n");
        ssize_t ssz = read(STDIN_FILENO, buf, sizeof(buf));
        printf("read from fd 0, result '%c' ssz=%zd\n", *buf, ssz);

        printf("exiting\n");
        exit(0);
}
