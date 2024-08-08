#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char const* argv[argc]) {
    if (argc != 2) {
        fprintf(stderr, "showip hostname\n");
        exit(1);
    }

    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,     // don't care IPv4 or IPv6
        .ai_socktype = SOCK_STREAM, // TCP stream sockets
    };

    struct addrinfo* res;
    int status = getaddrinfo(argv[1], 0, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(2);
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    char const*const ipver_f[2] = {"IPv4", "IPv6"};
    int ipver_i = 0;
    char ipstr[INET6_ADDRSTRLEN] = {0};
    for (struct addrinfo* p = res; p != 0; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
            inet_ntop(p->ai_family, &ipv4->sin_addr, ipstr, sizeof ipstr);
            ipver_i = 0;
        } else {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)(p->ai_addr);
            inet_ntop(p->ai_family, &ipv6->sin6_addr, ipstr, sizeof ipstr);
            ipver_i = 1;
        }
        printf("\t%s: %s\n", ipver_f[ipver_i], ipstr);
    }

    freeaddrinfo(res);

    return 0;
}
