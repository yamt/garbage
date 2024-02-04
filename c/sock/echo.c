#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main()
{
        int s = socket(PF_INET, SOCK_STREAM, 0);
        if (s == -1) {
                printf("socket failed\n");
                exit(1);
        }
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = htons(10000);
        if (bind(s, (const struct sockaddr *)&sin, sizeof(sin))) {
                printf("bind failed\n");
                exit(1);
        }
        printf("listening..\n");
        if (listen(s, 8)) {
                printf("listen failed\n");
                exit(1);
        }
        printf("accepting..\n");
        struct sockaddr_in sin_peer;
        socklen_t sin_peer_len = sizeof(sin_peer);
        int s2 = accept(s, (struct sockaddr *)&sin_peer, &sin_peer_len);
        if (s2 == -1) {
                printf("accept failed\n");
                exit(1);
        }
        printf("accepted a connection from %s:%u\n",
               inet_ntoa(sin_peer.sin_addr), (unsigned int)sin_peer.sin_port);
        while (true) {
                char buf[100];
                printf("recving..\n");
                ssize_t ssz = recv(s2, buf, sizeof(buf), 0);
                if (ssz == -1) {
                        printf("recv failed\n");
                        exit(1);
                }
                if (ssz == 0) {
                        break;
                }
                ssize_t to_send = ssz;
                ssize_t sent = 0;
                while (sent < to_send) {
                        printf("sending..\n");
                        ssz = send(s2, buf + sent, to_send - sent, 0);
                        if (ssz == -1) {
                                printf("send failed\n");
                                exit(1);
                        }
                        sent += ssz;
                }
        }
        printf("done\n");
}
