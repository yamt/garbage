#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int debug = 0;

int
set_nonblocking(int fd, bool nonblocking, bool *orig)
{
        int flags = fcntl(fd, F_GETFL, 0);
        int newflags = flags & ~O_NONBLOCK;
        if (nonblocking) {
                newflags |= O_NONBLOCK;
        }
        int ret = 0;
        if (flags != newflags) {
                ret = fcntl(fd, F_SETFL, newflags);
                if (ret == -1) {
                        ret = errno;
                        assert(ret > 0);
                }
        }
        if (orig != NULL) {
                *orig = (flags & O_NONBLOCK) != 0;
        }
        return ret;
}

struct xfer_state {
        int infd;
        int outfd;
        char buf[8];
        unsigned int start;
        unsigned int end;
};

int
xfer(struct xfer_state *st, struct pollfd **pfdp)
{
        if (debug) {
                fprintf(stderr, "xfer (%d->%d)\n", st->infd, st->outfd);
        }
        while (true) {
                while (st->start < st->end) {
                        ssize_t wsz = write(st->outfd, st->buf + st->start,
                                            st->end - st->start);
                        if (wsz == -1) {
                                if (errno == EAGAIN) {
                                        if (debug) {
                                                fprintf(stderr,
                                                        "write (%d) EAGAIN\n",
                                                        st->outfd);
                                        }
                                        struct pollfd *pfd = (*pfdp)++;
                                        pfd->fd = st->outfd;
                                        pfd->events = POLLOUT;
                                        return 0;
                                }
                                if (debug) {
                                        fprintf(stderr,
                                                "write (%d) error: %s\n",
                                                st->outfd, strerror(errno));
                                }
                                goto fail;
                        }
                        if (debug) {
                                fprintf(stderr, "write (%d) -> %zu\n",
                                        st->outfd, wsz);
                        }
                        st->start += wsz;
                }

                size_t sz = sizeof(st->buf);
                ssize_t rsz;
                rsz = read(st->infd, st->buf, sz);
                if (rsz == -1) {
                        if (errno == EAGAIN) {
                                if (debug) {
                                        fprintf(stderr, "read (%d) EAGAIN\n",
                                                st->infd);
                                }
                                struct pollfd *pfd = (*pfdp)++;
                                pfd->fd = st->infd;
                                pfd->events = POLLIN;
                                return 0;
                        }
                        if (debug) {
                                fprintf(stderr, "read (%d) error: %s\n",
                                        st->infd, strerror(errno));
                        }
                        goto fail;
                }
                if (rsz == 0) {
                        /* EOF */
                        if (debug) {
                                fprintf(stderr, "read (%d) EOF\n", st->infd);
                        }
                        goto fail;
                }
                if (debug) {
                        fprintf(stderr, "read (%d) -> %zu\n", st->infd, rsz);
                }
                st->start = 0;
                st->end = rsz;
        }
        return 0;
fail:
        return -1;
}

void
xfer_init(struct xfer_state *st, int infd, int outfd)
{
        memset(st, 0, sizeof(*st));
        st->infd = infd;
        st->outfd = outfd;
}

int
main(int argc, char **argv)
{
        if (argc != 3) {
                fprintf(stderr, "unexpected argc\n");
                exit(2);
        }
        const char *devname = argv[1];
        const char *rate_str = argv[2];
        unsigned int rate;
        rate = atoi(rate_str);
        if (rate == 0) {
                fprintf(stderr, "invalid rate: %s\n", rate_str);
                exit(2);
        }
        int fd;
        if (debug) {
                fprintf(stderr, "opening: %s\n", devname);
        }
        fd = open(devname, O_RDWR | O_NDELAY | O_NOCTTY);
        if (fd == -1) {
                fprintf(stderr, "open failed: %s: %s\n", devname,
                        strerror(errno));
                exit(1);
        }
        if (debug) {
                fprintf(stderr, "opened: %s\n", devname);
        }
        struct termios c;
        if (tcgetattr(fd, &c) < 0) {
                fprintf(stderr, "tcgetattr failed: %s: %s\n", devname,
                        strerror(errno));
                exit(1);
        }
        speed_t speed;
#if defined(__APPLE__)
        speed = rate;
#else
#error not implemented
#endif
        if (cfsetspeed(&c, speed) < 0) {
                fprintf(stderr, "cfsetspeed failed: %s: %s\n", devname,
                        strerror(errno));
                exit(1);
        }
#if 0
        /* 8 bit */
        c.c_cflag = (c.c_cflag & ~CSIZE) | CS8;
        /* no parity */
        c.c_cflag &= ~(PARENB | PARODD);
        /* stop bit 1 */
        c.c_cflag &= ~CSTOPB;
#endif
        /* the modem status lines */
        c.c_cflag |= CLOCAL;
        /* disable echo */
        c.c_cflag &= ~(ECHO | ECHONL);

#if 0
        c.c_cc[VMIN] = 1;
        c.c_cc[VTIME] = 0;
#endif

        if (tcsetattr(fd, TCSAFLUSH, &c) < 0) {
                fprintf(stderr, "tcsetattr failed: %s: %s\n", devname,
                        strerror(errno));
                exit(1);
        }
#if 1 
        if (set_nonblocking(fd, true, NULL) ||
            set_nonblocking(STDIN_FILENO, true, NULL) ||
            set_nonblocking(STDOUT_FILENO, true, NULL)) {
                fprintf(stderr, "set_nonblocking failed\n");
                exit(1);
        }
#endif
        struct xfer_state in_state;
        struct xfer_state out_state;
        xfer_init(&in_state, STDIN_FILENO, fd);
        xfer_init(&out_state, fd, STDOUT_FILENO);
        if (debug) {
                fprintf(stderr, "starting i/o loop\n");
        }
        while (true) {
                struct pollfd pfds[4];
                struct pollfd *pfd = pfds;
                if (xfer(&in_state, &pfd) || xfer(&out_state, &pfd)) {
                        exit(1);
                }
                assert(pfd >= pfds + 2);
                if (debug) {
                        unsigned int n = pfd - pfds;
                        fprintf(stderr, "poll (%u)\n", n);
                }
                int ret = poll(pfds, pfd - pfds, -1);
                if (ret == -1) {
                        fprintf(stderr, "poll failed: %s\n", strerror(errno));
                        exit(1);
                }
        }
}
