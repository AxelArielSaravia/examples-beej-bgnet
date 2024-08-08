#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>

#define STDIN 0 //file descriptor for standard input

int main() {
    struct timeval tv = {
        .tv_sec = 2,
        .tv_usec = 500000, //0.5s
    };

    fd_set readfds = {0};
    FD_SET(STDIN, &readfds);

    select(STDIN+1, &readfds, (void*)0, (void*)0, &tv);
    if (FD_ISSET(STDIN, &readfds)) {
        printf("A key was pressed!\n");
    } else {
        printf("Time out.\n");
    }
    return 0;
}
