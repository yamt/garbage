/*
 * a naive insecure HTTP 1.0 server for testing
 *
 * stdin is a listening socket.
 * see ../../listenexec
 */

#define _POSIX_C_SOURCE /* strerror_r */
#include <sys/socket.h>

#include <arpa/inet.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(THREADS)
#include <pthread.h>
#endif

#define LF 0x0a
#define CR 0x0d

char working_dir_store[PATH_MAX + 1];
const char *working_dir;
atomic_int count;
int nrequests = -1;

struct state {
        int fd;
        void *buf;
        char *path;
        size_t buflen;
        size_t offset;
};

int
try_read(struct state *state)
{
        ssize_t ssz;
        int ret;
        if (state->buflen == state->offset) {
                size_t newlen;
                if (state->buflen == 0) {
                        newlen = 100;
                } else {
                        newlen = state->buflen * 2;
                }
                void *newbuf = realloc(state->buf, newlen);
                if (newbuf == NULL) {
                        return ENOMEM;
                }
                state->buf = newbuf;
                state->buflen = newlen;
        }
        ssz = read(state->fd, state->buf + state->offset,
                   state->buflen - state->offset);
        if (ssz == -1) {
                ret = errno;
                assert(ret > 0);
                return ret;
        }
        if (ssz == 0) {
                printf("unexpected EOF\n");
                return ECONNRESET;
        }
        assert(ssz >= 0);
        state->offset += ssz;
        return 0;
}

int
try_parse_line(const char *p, const char **npp, size_t len)
{
        size_t i;
        for (i = 0; i + 1 < len; i++) {
                if (p[i] == CR && p[i + 1] == LF) {
                        *npp = &p[i + 2];
                        return 0;
                }
        }
        return EAGAIN;
}

int
try_parse(struct state *state)
{
        const char *p = state->buf;
        size_t len = state->offset;
        int ret;

        const char *np;
        ret = try_parse_line(p, &np, len);
        if (ret != 0) {
                return ret;
        }

        /* the line looks like "GET / HTTP/1.1" */
        const char *cp;
        const char *sp = NULL;
        const char *ep = NULL;
        for (cp = p; cp < np; cp++) {
                if (*cp == ' ') {
                        if (sp == NULL) {
                                sp = cp + 1;
                        } else if (ep == NULL) {
                                ep = cp + 1;
                        } else {
                                return EINVAL;
                        }
                }
        }
        if (sp == NULL || ep == NULL) {
                return EINVAL;
        }
        if (sp - p != 4 || strncmp(p, "GET ", 4)) {
                return EINVAL;
        }
        if (np - ep != 8 + 2) { /* do not care 1.0 or 1.1 */
                return EINVAL;
        }

        /* skip the rest of the request */
        while (true) {
                len -= np - p;
                p = np;
                ret = try_parse_line(p, &np, len);
                if (ret != 0) {
                        return ret;
                }
                if (p == np - 2) {
                        break;
                }
        }

        assert(state->path == NULL);
        ep--; /* trim a space */
        state->path = malloc(ep - sp + 1);
        if (state->path == NULL) {
                return ENOMEM;
        }
        memcpy(state->path, sp, ep - sp);
        state->path[ep - sp] = 0;
        return 0;
}

int
open_file(struct state *state, int *fdp)
{
        /*
         * note: we don't care security
         * ("..", symlinks, etc)
         */
        assert(state->path != NULL);
        const char *path = state->path;
        int ret;
        while (*path == '/') {
                path++;
        }
        if (*path == 0) {
                return EACCES;
        }
        int fd = open(path, O_RDONLY);
        if (fd == -1) {
                ret = errno;
                assert(ret > 0);
                return ret;
        }
        *fdp = fd;
        return 0;
}

int
send_file(struct state *state, int fd)
{
        int ret;
        while (true) {
                char buf[1000];
                ssize_t ssz;
                ssz = read(fd, buf, sizeof(buf));
                if (ssz == -1) {
                        ret = errno;
                        assert(ret > 0);
                        goto fail;
                }
                if (ssz == 0) {
                        break;
                }
                size_t offset = 0;
                while (offset < ssz) {
                        ssize_t wssz;
                        wssz = write(state->fd, buf + offset, ssz - offset);
                        if (wssz == -1) {
                                ret = errno;
                                assert(ret > 0);
                                goto fail;
                        }
                        offset += wssz;
                }
        }
        return 0;
fail:
        return ret;
}

int
send_response(struct state *state)
{
        char response[100];
        ssize_t ssz;
        int ret;
        int fd = -1;

        ret = open_file(state, &fd);
        if (ret == ENOENT) {
                ret = snprintf(response, sizeof(response),
                               "HTTP/1.0 404 Not Found\r\n\r\n");
        } else if (ret != 0) {
                ret = snprintf(response, sizeof(response),
                               "HTTP/1.0 500 Internal Server Error\r\n\r\n");
        } else {
                ret = snprintf(response, sizeof(response),
                               "HTTP/1.0 200 OK\r\n\r\n");
        }
        assert(ret > 0 && ret < sizeof(response));
        ssz = write(state->fd, response, ret);
        if (ssz == -1) {
                ret = errno;
                assert(ret > 0);
                return ret;
        }
        ret = 0;
        if (fd != -1) {
                ret = send_file(state, fd);
        }
        if (fd != -1) {
                (void)close(fd);
        }
        return ret;
}

void
reset(struct state *state)
{
    free(state->path);
	free(state->buf);
    int fd = state->fd;
    memset(state, 0, sizeof(*state));
    state->fd = fd;
}

int
do_io(int fd)
{
        struct state state0;
        struct state *state = &state0;
        memset(state, 0, sizeof(*state));
        char errbuf[100];
        state->fd = fd;

        int ret;
        while (true) {
                ret = try_parse(state);
                if (ret == EAGAIN) {
                        ret = try_read(state);
                        if (ret != 0) {
                                strerror_r(ret, errbuf, sizeof(errbuf));
                                printf("try_read failed: %s\n", errbuf);
                                break;
                        }
                        continue;
                }
                if (ret != 0) {
                        break;
                }
                ret = send_response(state);
                if (ret != 0) {
                        strerror_r(errno, errbuf, sizeof(errbuf));
                        printf("send_response failed: %s\n", errbuf);
                        break;
                }
#if 0
                struct linger l;
                l.l_onoff = 1;
                l.l_linger = 0;
                ret = setsockopt(state->fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
                if (ret != 0) {
                        ret = errno;
                        assert(ret > 0);
                        strerror_r(errno, errbuf, sizeof(errbuf));
                        printf("setsockopt SO_LINGER failed: %s\n", errbuf);
                        break;
                }
#endif
                ret = close(state->fd);
                if (ret != 0) {
                        ret = errno;
                        assert(ret > 0);
                        strerror_r(errno, errbuf, sizeof(errbuf));
                        printf("close failed: %s\n", errbuf);
                        break;
                }
                break;
        }
        reset(state);

        int n = atomic_fetch_add(&count, 1) + 1;
        if (nrequests != -1 && n == nrequests) {
                printf("processed %u/%u requests\n", n, nrequests);
                exit(0);
        }
        if ((n % 200) == 0) {
                if (nrequests == -1) {
                        printf("processed %u requests\n", n);
                } else {
                        printf("processed %u/%u requests\n", n, nrequests);
                }
        }
        return ret;
}

int
do_accept(int listenfd)
{
        struct sockaddr_storage ss;
        struct sockaddr *sa = (void *)&ss;
        socklen_t slen;
        int ret;

        slen = sizeof(ss);
        int fd = accept(listenfd, sa, &slen);
        if (fd == -1) {
                ret = errno;
                assert(ret > 0);
                perror("accept");
                return ret;
        }
        assert(fd >= 0);
#if 0
        /* Note: wasi-libc fills the peer address with a dummy */
        if (sa->sa_family == AF_INET) {
                const struct sockaddr_in *sin = (const void *)sa;
                char buf[INET_ADDRSTRLEN];
                printf("accepted INET: %s %u\n",
                       inet_ntop(sa->sa_family, sa, buf, sizeof(buf)),
                       ntohs(sin->sin_port));
        } else if (sa->sa_family == AF_INET6) {
                const struct sockaddr_in6 *sin6 = (const void *)sa;
                char buf[INET6_ADDRSTRLEN];
                printf("accepted INET6: %s %u\n",
                       inet_ntop(sa->sa_family, sa, buf, sizeof(buf)),
                       ntohs(sin6->sin6_port));
        } else {
                printf("accepted\n");
        }
#endif
        return do_io(fd);
}

int
do_loop(int listenfd)
{
        int ret;
        while (true) {
                ret = do_accept(listenfd);
                if (ret == ECONNRESET) {
                    continue;
                }
                if (ret != 0) {
                        break;
                }
        }
        return ret;
}

#if defined(THREADS)
void *
thread_start(void *vp)
{
        int fd = STDIN_FILENO;
        int ret;
        printf("thread %p starting\n", (const void *)pthread_self());
        ret = do_loop(fd);
        printf("thread %p exiting with %u\n", (const void *)pthread_self(),
               ret);
        return (void *)(uintptr_t)ret;
}
#endif

int
main(int argc, char **argv)
{
        int ch;
        while ((ch = getopt(argc, argv, "n:")) != -1) {
                switch (ch) {
                case 'n':
                        nrequests = atoi(optarg);
                        break;
                }
        }
        atomic_init(&count, 0);

        int ret;
        working_dir = getcwd(working_dir_store, sizeof(working_dir_store));
        if (working_dir == NULL) {
                ret = errno;
                assert(errno > 0);
                printf("getcwd failed: %s\n", strerror(ret));
                exit(1);
        }
        printf("working_dir: %s\n", working_dir);
#if defined(THREADS)
        int n = 8;
        pthread_t t[n];
		pthread_attr_t a;
        ret = pthread_attr_init(&a);
        if (ret != 0) {
            goto fail;
        }
        ret = pthread_attr_setstacksize(&a, 4096);
        if (ret != 0) {
            goto fail;
        }
        int i;
        for (i = 0; i < n; i++) {
                ret = pthread_create(&t[i], &a, thread_start, NULL);
                if (ret != 0) {
                        printf("pthread_create failed ret=%d (%s)\n", ret, strerror(ret));
                        break;
                }
        }
        ret = pthread_attr_destroy(&a);
        assert(ret == 0);
        n = i;
        int ret1 = 0;
        for (i = 0; i < n; i++) {
                void *value;
                ret = pthread_join(t[i], &value);
                assert(ret == 0);
                ret = (uintptr_t)value;
                if (ret1 == 0 && ret != 0) {
                        ret1 = ret;
                }
        }
        if (ret1 != 0) {
            ret = ret1;
        }
fail:;
#else
        int fd = STDIN_FILENO;
        ret = do_loop(fd);
#endif
        printf("exiting ret=%d (%s)\n", ret, strerror(ret));
        exit(1);
}
