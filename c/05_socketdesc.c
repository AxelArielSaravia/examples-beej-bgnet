#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main() {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo* res;
    int status = getaddrinfo("www.example.com","http", &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(2);
    }
    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == -1) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(2);
    }

    printf("Socket descriptor: %d\n", s);

    freeaddrinfo(res);
    return 0;
}
