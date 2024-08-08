#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490"
#define MAX_DATA_SIZE 127 //max number of bytes we can get at once

int main(int argc, char const* argv[argc]) {
    if (argc != 2) {
        fprintf(stderr, "usage:\n\tclient hostname\n");
        exit(1);
    }
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo* servinfo = 0;
    int status = getaddrinfo(argv[1], PORT, &hints, &servinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int addr_family = -1;
    int sockfd = -1;
    _Bool search_addrinfo = 1;
    struct addrinfo* ssi = servinfo;
    while (ssi != 0 && search_addrinfo) {
        addr_family = ssi->ai_family;
        sockfd = socket(addr_family, ssi->ai_socktype, ssi->ai_protocol);
        if (sockfd == -1) {
            perror("client: socket");
        } else if (connect(sockfd, ssi->ai_addr, ssi->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
        } else {
            search_addrinfo = 0;
        }

        if (search_addrinfo) {
            ssi = ssi->ai_next;
        }
    }

    if (ssi == 0) {
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }

    char addr_str[INET6_ADDRSTRLEN] = {0};
    if (addr_family == AF_INET) {
        struct in_addr* addr4 = &((struct sockaddr_in*)ssi)->sin_addr;
        inet_ntop(addr_family, addr4, addr_str, sizeof addr_str);
    } else {
        struct in6_addr* addr6 = &((struct sockaddr_in6*)ssi)->sin6_addr;
        inet_ntop(addr_family, addr6, addr_str, sizeof addr_str);
    }
    printf("client: connection to %s\n", addr_str);

    freeaddrinfo(servinfo);
    servinfo = 0;

    char recv_buf[MAX_DATA_SIZE] = {0};
    int numbytes = recv(sockfd, recv_buf, MAX_DATA_SIZE-1, 0);
    if (numbytes == -1) {
        perror("recv");
        exit(1);
    }
    printf("client: received '%s'\n", recv_buf);
    close(sockfd);

    return 0;
}
