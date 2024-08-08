#include <stdio.h>
#include <poll.h>

int main() {
    struct pollfd pfds[1] = {{
        .fd = 0,
        .events = POLLIN
    }};

    printf("Hit RETURN or wait 2.5 seconds for timeout\n");

    int num_events = poll(pfds, 1, 2500);

    if (num_events == 0) {
        printf("Poll time out!\n");
    } else {
        if ((pfds[0].revents & POLLIN) == POLLIN) {
            printf("File descriptor %d is ready to read\n", pfds[0].fd);
        } else {
            printf("Unexpected event occurred: %d\n", pfds[0].revents);
        }
    }
    return 0;
}
