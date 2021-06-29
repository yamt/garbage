
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
configure_keepalive(int s)
{
	int level = IPPROTO_TCP;
	unsigned int uval;
	int sval;
	int ret;

	sval = 1;
	ret = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &sval, sizeof(sval));
	if (ret != 0) {
		errx(EXIT_FAILURE, "setsockopt SO_KEEPALIVE");
	}

#if !defined(TCP_KEEPALIVE)
/*
 * TCP_KEEPIDLE - netbsd
 * TCP_KEEPALIVE - macos
 */
#define TCP_KEEPALIVE TCP_KEEPIDLE
#endif

	uval = 7;
	ret = setsockopt(s, level, TCP_KEEPALIVE, &uval, sizeof(uval));
	if (ret != 0) {
		errx(EXIT_FAILURE, "setsockopt TCP_KEEPALIVE");
	}

	uval = 3;
	ret = setsockopt(s, level, TCP_KEEPINTVL, &uval, sizeof(uval));
	if (ret != 0) {
		errx(EXIT_FAILURE, "setsockopt TCP_KEEPINTVL");
	}
}

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
		s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (s < 0)
			continue;
		if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
			close(s);
			s = -1;
			continue;
		}
		break;
	}
	freeaddrinfo(res0);

	if (s < 0)
		errx(EXIT_FAILURE, "no connectable address");

	configure_keepalive(s);

	while (1) {
#if 0
		ssize_t n;
		char buf[256];

		n = read(STDIN_FILENO, buf, sizeof(buf));
		if (n == (ssize_t)-1)
			err(EXIT_FAILURE, "read");
		if (n == 0)
			break;
		while (n > 0) {
			ssize_t written = write(s, buf, n);

			if (written == (ssize_t)-1)
				err(EXIT_FAILURE, "write");
			n -= written;
		}
#else
		sleep(10);
#endif
	}

	close(s);

	exit(EXIT_SUCCESS);
	/* NOTREACHED */
}
