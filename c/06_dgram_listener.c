#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "4950"
#define MAX_BUF_LEN 100

int main() {
    struct addrinfo hints = {
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_PASSIVE,
    };
    struct addrinfo* servinfo = 0;
    int status = getaddrinfo(0, PORT, &hints, &servinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int sockfd = -1;
    _Bool search_addrinfo = 1;
    struct addrinfo* ssi = servinfo;
    while (ssi != 0 && search_addrinfo) {
        sockfd = socket(PF_INET6, ssi->ai_socktype, ssi->ai_protocol);
        if (sockfd == -1) {
            perror("listener: socket");
        } else if (bind(sockfd, ssi->ai_addr, ssi->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
        } else {
            search_addrinfo = 0;
        }
        if (search_addrinfo) {
            ssi = ssi->ai_next;
        }
    }
    if (ssi == 0) {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("listener: waiting to recvfrom...\n");

    struct sockaddr_storage recv_addr = {0};
    socklen_t addr_len = sizeof recv_addr;
    char recv_buf[MAX_BUF_LEN] = {0};
    int numbytes = recvfrom(
        sockfd,
        recv_buf,
        MAX_BUF_LEN - 1,
        0,
        (struct sockaddr*)&recv_addr,
        &addr_len
    );
    if (numbytes == -1) {
        perror("recvfrom");
        exit(1);
    }
    char addr_str[INET6_ADDRSTRLEN] = {0};
    {
        struct in6_addr* addr6 = &((struct sockaddr_in6*)&recv_addr)->sin6_addr;
        inet_ntop(AF_INET6, addr6, addr_str, sizeof addr_str);
    }
    printf("listener: got packet from %s\n", addr_str);
    printf("listener: packet is %d bytes long\n", numbytes);
    printf("listener: packet contains \"%s\"\n", recv_buf);
    close(sockfd);

    return 0;
}
