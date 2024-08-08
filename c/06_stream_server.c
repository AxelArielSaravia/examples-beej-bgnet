#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490" //the port users will be connecting to
#define BACKLOG 10 //how meny pending connections queue will hold

void sigchld_handler(int s) {
    int saved_errno = errno;
    //waitpid() might overwrite errno, so we save and restore it
    while (waitpid(-1, 0, WNOHANG) > 0);
    errno = saved_errno;
}

int main() {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE, //use my ip
    };

    struct addrinfo* servinfo = 0;
    int status = getaddrinfo(0, PORT, &hints, &servinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int sockfd = -1;

    int yes = 1;
    _Bool search_addrinfo = 1;
    struct addrinfo* ssi = servinfo;
    while (ssi != 0 && search_addrinfo) {
        sockfd = socket(ssi->ai_family, ssi->ai_socktype, ssi->ai_protocol);
        printf("data: %d\n", ssi->ai_addr->sa_family);
        if (sockfd == -1) {
            perror("server: socket");
        } else if (
            setsockopt(
                sockfd,
                SOL_SOCKET,
                SO_REUSEADDR,
                &yes,
                sizeof yes
            ) == -1
        ) {
            perror("setsockopt");
            exit(1);
        } else if (bind(sockfd, ssi->ai_addr, ssi->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
        } else {
            search_addrinfo = 0;
        }

        if (search_addrinfo) {
            ssi = ssi->ai_next;
        }
    }

    freeaddrinfo(servinfo); //all done with this structure
    servinfo = 0;

    if (ssi == 0) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    struct sigaction sa = {.sa_handler = sigchld_handler};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("server: waiting for connections...\n");

    char addr_str[INET6_ADDRSTRLEN] = {0};

    //main accept() loop
    while (1) {
        struct sockaddr_storage send_addr = {0};
        socklen_t sin_size = sizeof send_addr;

        int send_sockfd = accept(sockfd, (struct sockaddr*)&send_addr, &sin_size);
        if (send_sockfd == -1) {
            perror("accept");
        } else {
            if (send_addr.ss_family == AF_INET) {
                struct in_addr* addr4 = &((struct sockaddr_in*)&send_addr)->sin_addr;
                inet_ntop(send_addr.ss_family, addr4, addr_str, sizeof addr_str);
            } else {
                struct in6_addr* addr6 = &((struct sockaddr_in6*)&send_addr)->sin6_addr;
                inet_ntop(send_addr.ss_family, addr6, addr_str, sizeof addr_str);
            }
            printf("server: got connection from %s\n", addr_str);

            if (!fork()) { //this is the child process
                close(sockfd);// child doesn't need the listener

                if (send(send_sockfd, "Hello, world!\n", 13, 0) == -1) {
                    perror("send");
                }
                close(send_sockfd);
                exit(0);
            }
            close(send_sockfd); //parent doesn't need this
        }
    }

    return 0;
}
