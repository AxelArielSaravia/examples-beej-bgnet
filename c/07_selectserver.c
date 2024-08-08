#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT "3490"

void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &((struct sockaddr_in*)sa)->sin_addr;
    }
    return &((struct sockaddr_in6*)sa)->sin6_addr;
}

int main() {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };
    struct addrinfo* addrinfo = &(struct addrinfo){0};
    int status = getaddrinfo(0, PORT, &hints, &addrinfo);
    if (status == -1) {
        fprintf(stderr, "ERROR getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    int listener = -1;
    struct addrinfo* temp_ai = addrinfo;
    for (;;) {
        if (temp_ai == 0) {
            fprintf(stderr, "selectserver: failed to bind\n");
            exit(2);
        }
        listener = socket(temp_ai->ai_family, temp_ai->ai_socktype, temp_ai->ai_protocol);
        if (listener > -1) {
            int yes = 1;
            setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

            if (bind(listener, temp_ai->ai_addr, temp_ai->ai_addrlen) == -1) {
                close(listener);
            } else {
                break;
            }
        }

        temp_ai = temp_ai->ai_next;
    }
    freeaddrinfo(addrinfo);
    addrinfo = 0;
    temp_ai = 0;

    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    fd_set master = {0}; //master file descriptor list
    fd_set read_fds = {0}; //temp file descriptor list for select()

    FD_SET(listener, &master);

    //keep track of the biggest file descriptor
    int fdmax = listener;

    #define BUF_SIZE 256
    char buf[BUF_SIZE] = {0};

    char remote_ip[INET6_ADDRSTRLEN] = {0};

    for (;;) {
        read_fds = master; //copy it
        if (select(fdmax + 1, &read_fds, (void*)0, (void*)0, (void*)0) == -1) {
            perror("select");
            exit(4);
        }

        //run through the existing connections looking for data to read
        for (int i = 0; i <= fdmax; i += 1) {
            if (!FD_ISSET(i, &read_fds)) {
                continue;
            }
            if (i == listener) { //handle new connections
                struct sockaddr_storage remoteaddr = {0};
                socklen_t remoteaddr_len = sizeof remoteaddr;
                int newfd = accept(
                    listener,
                    (struct sockaddr*)&remoteaddr,
                    &remoteaddr_len
                );
                if (newfd == -1) {
                    perror("accept");
                } else {
                    FD_SET(newfd, &master);
                    if (newfd > fdmax) {
                        fdmax = newfd;
                    }
                    printf(
                        "selectserver: new connection from %s on socket %d\n",
                        inet_ntop(
                            remoteaddr.ss_family,
                            get_in_addr((struct sockaddr*)&remoteaddr),
                            remote_ip,
                            sizeof remote_ip
                        ),
                        newfd
                    );
                }
            } else {
                int nbytes = recv(i, buf, BUF_SIZE-1, 0);
                if (nbytes <= 0) {
                    if (nbytes == 0) { //connection closed
                        printf("selectserver: socket %d hung up\n", i);
                    } else {
                        perror("recv");
                    }
                    close(i);
                    FD_CLR(i, &master);
                } else { //we got some data from client
                    for (int j = 0; j <= fdmax; j += 1) {
                        if (!FD_ISSET(j, &master)) {
                            continue;
                        }
                        if (j != listener && j != i) {
                            if (send(j, buf, nbytes, 0) == -1) {
                                perror("send");
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
