#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main()
{
        int pipefds[2];
        int ret;
        ret = pipe(pipefds);
        if (ret != 0) {
                printf("pipe failed\n");
                exit(1);
        }
        pid_t p = fork();
        if (p == -1) {
                printf("fork failed\n");
                exit(1);
        }
        if (p == 0) {
                /* sleep to ensure the parent starts poll before we exit */
                sleep(1);
                _Exit(0);
        }
        ret = close(pipefds[0]); /* read side */
        if (ret != 0) {
                printf("close failed unexpectdly\n");
                exit(1);
        }
        struct pollfd pfd;
        memset(&pfd, 0, sizeof(pfd));
        pfd.fd = pipefds[1]; /* write side */
        pfd.events = POLLHUP;
        ret = poll(&pfd, 1, 3000);
        if (ret != 1) {
                printf("poll returned unexpectd value %d\n", ret);
                exit(1);
        }
        printf("success\n");
        exit(0);
}
