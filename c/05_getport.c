#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// int main() {
//     struct addrinfo hints = {
//         .ai_flags = AI_PASSIVE,
//         .ai_family = AF_UNSPEC,
//         .ai_socktype = SOCK_STREAM,
//     };
//     struct addrinfo* res;
//     int status = getaddrinfo(0,"3490", &hints, &res);
//     if (status != 0) {
//         fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
//         exit(2);
//     }
//
//     int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//     if (sockfd == -1) {
//         perror("socket error:");
//         exit(2);
//     }
//
//     if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
//         perror("bind error:");
//         exit(2);
//     }
//
//     freeaddrinfo(res);
//
//     return 0;
// }

// Old way
#define MYPORT 3490
int main() {
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket error:");
        exit(2);
    }

    struct sockaddr_in my_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(MYPORT),
        .sin_addr = {.s_addr = INADDR_ANY},
        .sin_zero = {0},
    };

    if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof my_addr) == -1) {
        perror("bind error:");
    }

    return 0;
}
