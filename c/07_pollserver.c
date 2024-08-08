#include <sys/poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT "3490"
#define MAX_CLIENT_DATA 256

#define MIN_PFD_SET_LEN 8
typedef struct Pfd_Set Pfd_Set;
struct Pfd_Set {
    unsigned      cap;
    unsigned      len;
    struct pollfd* pfds;
};

Pfd_Set pfd_set_create(unsigned cap) {
    if (cap < MIN_PFD_SET_LEN) {
        cap = MIN_PFD_SET_LEN;
    }
    struct pollfd* pfds = malloc((sizeof *pfds) * MIN_PFD_SET_LEN);
    if (!pfds) {
        fprintf(stderr, "Alloc ERROR: Buy more RAM\n");
        exit(1);
    }
    return (Pfd_Set){
        .cap = MIN_PFD_SET_LEN,
        .len = 0,
        .pfds = pfds,
    };
}

void pfd_set_add(Pfd_Set ps[static 1], int newfd) {
    Pfd_Set t = *ps;
    if (t.len == t.cap) {
        t.cap *= 2;
        struct pollfd* pfds = realloc(t.pfds, (sizeof *pfds) * t.cap);
        if (!pfds) {
            fprintf(stderr, "Alloc ERROR: Buy more RAM\n");
            exit(1);
        }
        t.pfds = pfds;
    }
    t.pfds[t.len] = (struct pollfd){
        .fd = newfd,
        .events = POLLIN,
    };
    t.len += 1;
    *ps = t;
}

void pfd_set_del(Pfd_Set ps[static 1], unsigned i) {
    Pfd_Set t = *ps;
    if (i >= t.len) {
        return;
    }
    t.pfds[i] = t.pfds[t.len-1];
    t.pfds[t.len-1] = (struct pollfd){0};
    t.len -= 1;
}

void pfd_set_destroy(Pfd_Set ps[static 1]) {
    free((*ps).pfds);
    *ps = (Pfd_Set){0};
}

int get_listener_socket(void) {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };
    struct addrinfo* ai = &(struct addrinfo){0};
    int rv = getaddrinfo((void*)0, PORT, &hints, &ai);
    if (rv != 0) {
        fprintf(stderr, "Select server: %s\n", gai_strerror(rv));
        exit(1);
    }
    int listener = -1;
    struct addrinfo* p = ai;
    while (p != (void*)0) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            p = p->ai_next;
        }
        //Lose the pesky "address already in use" error message
        int yes = 0;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            p = p->ai_next;
        }
        break;
    }
    freeaddrinfo(ai);

    if (p == 0) {
        return -1;
    }
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}


int main() {
    Pfd_Set pfd_set = pfd_set_create(0);
    int listener = get_listener_socket();
    if (listener == -1) {
        fprintf(stderr, "ERROR: getting listening socket \n");
        exit(1);
    }
    printf("Listener socket %d\n",listener);
    pfd_set_add(&pfd_set, listener);

    char buf[MAX_CLIENT_DATA] = {0};
    char remoteIP[INET6_ADDRSTRLEN] = {0};
    for (;;) {
        int poll_count = poll(pfd_set.pfds, pfd_set.len, -1);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }
        //Run through the existing connections looking for data to read
        for (int i = 0; i < pfd_set.len; i += 1) {
            struct pollfd sender_pfd = pfd_set.pfds[i];
            //check if someon's ready to read
            if ((sender_pfd.revents & POLLIN) != POLLIN) {
                continue;
            }
            //If listener is ready to read, handle new connection
            if (sender_pfd.fd == listener) {
                struct sockaddr_storage remote_addr = {0};
                socklen_t addrlen = sizeof remote_addr;
                int newfd = accept(
                    listener,
                    (struct sockaddr*)&remote_addr,
                    &addrlen
                );
                if (newfd == -1) {
                    perror("accept");
                } else {
                    pfd_set_add(&pfd_set, newfd);
                    if (remote_addr.ss_family == AF_INET) {
                        inet_ntop(
                            remote_addr.ss_family,
                            &((struct sockaddr_in*)&remote_addr)->sin_addr,
                            remoteIP,
                            sizeof remoteIP
                        );
                    } else {
                        inet_ntop(
                            remote_addr.ss_family,
                            &((struct sockaddr_in6*)&remote_addr)->sin6_addr,
                            remoteIP,
                            sizeof remoteIP
                        );
                    }
                    printf(
                        "pollserver: new connection from %s on socket %d\n",
                        remoteIP,
                        newfd
                    );
                }
            } else {
                //If not the listener, we're just a regular client
                int nbytes = recv(sender_pfd.fd, buf, sizeof buf, 0);
                printf("bytes recive: %d\n", nbytes);
                if (nbytes <= 0) {
                    if (nbytes == 0) {
                        //connection closed
                        printf("pollserver: socket %d hung up\n", sender_pfd.fd);
                    } else {
                        perror("recv");
                    }
                    close(sender_pfd.fd);
                    pfd_set_del(&pfd_set, i);
                } else {
                    //We got some good data from a client
                    for (int j = 0; j < pfd_set.len; j += 1) {
                        //send to everyone
                        int dest_fd = pfd_set.pfds[j].fd;
                        //except the listener and ourselves
                        if (dest_fd != listener && dest_fd != sender_pfd.fd) {
                            if (send(dest_fd, buf, nbytes, 0) == -1) {
                                perror("send");
                            }
                        }
                    }
                } //End handle data from client
            } //End fo ready-to-read from poll()
        } //End looping through file descriptor
    }

    return 0;
}
