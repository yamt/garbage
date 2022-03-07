
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
        char *target;
        char *port;
        struct addrinfo hint, *res0, *res;
        int s;
        int error;

        if (argc != 3)
                errx(EXIT_FAILURE, "arg");
        target = argv[1];
        port = argv[2];

        printf("target: %s:%s\n", target, port);

        memset(&hint, 0, sizeof(hint));
        hint.ai_socktype = SOCK_STREAM;

        error = getaddrinfo(target, port, &hint, &res0);
        if (error)
                errx(EXIT_FAILURE, "getaddrinfo: %s", gai_strerror(error));

        if (res0->ai_canonname)
                printf("canonname: %s\n", res0->ai_canonname);

        for (res = res0; res; res = res->ai_next) {
                printf("family %u socktype %u, protocol %u\n",
                       (unsigned int)res->ai_family,
                       (unsigned int)res->ai_socktype,
                       (unsigned int)res->ai_protocol);
                // res->ai_addr, res->ai_addrlen
        }
        freeaddrinfo(res0);

        exit(EXIT_SUCCESS);
        /* NOTREACHED */
}
