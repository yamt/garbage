#include <sys/socket.h>
#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARRAYCOUNT(a) (sizeof(a) / sizeof(a[0]))

int
main(int argc, char **argv)
{
        const char *host = "localhost";
        const char *port = "";
        bool systemd_style = false;

        int ch;
        while ((ch = getopt(argc, argv, "h:p:s")) != -1) {
                switch (ch) {
                case 'h':
                        host = optarg;
                        break;
                case 'p':
                        port = optarg;
                        break;
                case 's':
                        systemd_style = true;
                        break;
                }
        }
        argc -= optind;
        argv += optind;
        if (argc == 0) {
                fprintf(stderr, "no command is specified\n");
                exit(2);
        }

        int sock[1];
        int nsock = 0;

        struct addrinfo hints, *res, *res0;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;

        int gai_res = getaddrinfo(host, port, &hints, &res0);
        if (gai_res != 0) {
                fprintf(stderr, "getaddrinfo failed: %s\n",
                        gai_strerror(gai_res));
                exit(1);
        }

        fprintf(stderr, "canonname: %s\n", res0->ai_canonname);

        for (res = res0; res && nsock < ARRAYCOUNT(sock); res = res->ai_next) {
                int s;

                fprintf(stderr, "family: %d, type: %d, protocol: %d\n",
                        res->ai_family, res->ai_socktype, res->ai_protocol);
                char hbuf[NI_MAXHOST];
                char sbuf[NI_MAXSERV];
                if (getnameinfo(res->ai_addr, res->ai_addrlen, hbuf,
                                sizeof(hbuf), sbuf, sizeof(sbuf),
                                NI_NUMERICHOST | NI_NUMERICSERV)) {
                        fprintf(stderr, "getnameinfo failed\n");
                        exit(1);
                }
                fprintf(stderr, "host: %s, serv: %s\n", hbuf, sbuf);
                s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (s < 0) {
                        fprintf(stderr, "socket failed: %s\n",
                                strerror(errno));
                        continue;
                }
                const int one = 1;
                if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one,
                               sizeof(one))) {
                        fprintf(stderr, "SO_REUSEADDR failed: %s\n",
                                strerror(errno));
                        /* not fatal */
                }
                if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
                        fprintf(stderr, "bind failed: %s\n", strerror(errno));
                        continue;
                }
                if (listen(s, 32) < 0) {
                        fprintf(stderr, "listen failed: %s\n",
                                strerror(errno));
                        continue;
                }
                sock[nsock] = s;
                nsock++;
        }
        fprintf(stderr, "nsock=%u\n", nsock);

        if (nsock == 0) {
                fprintf(stderr, "no usable socket\n");
                exit(1);
        }

        fprintf(stderr, "execute: %s\n", argv[0]);
        int listenfd = STDIN_FILENO;
        int ret;
        if (systemd_style) {
                listenfd = 3;
                ret = setenv("LISTEN_FDS", "1", 1);
                if (ret != 0) {
                        fprintf(stderr, "setenv failed with %d\n", errno);
                        exit(1);
                }
        }
        if (sock[0] != listenfd) {
                ret = dup2(sock[0], listenfd);
                if (ret == -1) {
                        fprintf(stderr, "dup failed with %d\n", errno);
                        exit(1);
                }
                ret = close(sock[0]);
                if (ret == -1) {
                        fprintf(stderr, "close failed with %d\n", errno);
                        exit(1);
                }
        }
        int nargs = argc;
        char **args = calloc(nargs + 1, sizeof(char *));
        if (args == NULL) {
                fprintf(stderr, "calloc\n");
                exit(1);
        }
        int i;
        for (i = 0; i < nargs; i++) {
                args[i] = argv[i];
        }
        args[nargs + 1] = NULL;
        ret = execvp(argv[0], args);
        assert(ret == -1);
        fprintf(stderr, "execve failed with %d\n", errno);
}
