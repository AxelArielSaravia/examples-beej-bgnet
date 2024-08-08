#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_PORT "4950"

int main(int argc, char const* argv[argc]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: talker hostname message\n");
        exit(1);
    }
    struct addrinfo hints = {
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_DGRAM,
    };
    struct addrinfo* servinfo = 0;
    int status = getaddrinfo(argv[1], SERVER_PORT, &hints, &servinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int sockdf = -1;
    _Bool search_addrinfo = 1;
    struct addrinfo* ssi = servinfo;
    while (ssi != 0 && search_addrinfo) {
        sockdf = socket(PF_INET6, ssi->ai_socktype, ssi->ai_protocol);
        if (sockdf == -1) {
            perror("talker: socket");
        } else {
            search_addrinfo = 0;
        }
        if (search_addrinfo) {
            ssi = ssi->ai_next;
        }
    }

    if (ssi == 0) {
        fprintf(stderr, "talker: failed to create a socket\n");
        exit(1);
    }

    size_t msg_len = strlen(argv[2]);

    int n = sendto(sockdf, argv[2], msg_len, 0, ssi->ai_addr, ssi->ai_addrlen);
    if (n == -1) {
        perror("sendto");
        exit(1);
    }
    freeaddrinfo(servinfo);
    servinfo = 0;
    close(sockdf);

    printf("talker: send %d bytes to %s\n", n, argv[1]);

    return 0;
}
