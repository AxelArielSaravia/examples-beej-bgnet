#include <stdlib.h>
#include <stdint.h>

#define VERSION 1

typedef int descriptor;

typedef struct sockaddr sockaddr;
struct sockaddr {
    unsigned short family;  //address family, AF_xxx
    unsigned char data[14]; //14 bytes of protocol address
};

typedef struct in4_addr in4_addr;
struct in4_addr {
    uint32_t addr;
};

typedef struct sockaddr_in4 sockaddr_in4;
struct sockaddr_in4 {
    short family;
    unsigned short port;
    in4_addr addr;
    unsigned char zero[8];
};

typedef struct in6_addr in6_addr;
struct in6_addr {
    unsigned char addr[16];
};

typedef struct sockaddr_in6 sockaddr_in6;
struct sockaddr_in6 {
    uint16_t family;
    uint16_t port;
    uint32_t flowinfo;
    in6_addr addr;
    uint32_t scope_id;
};

typedef struct addrinfo addrinfo;
struct addrinfo {
    addrinfo* next;  //linked list, next node
    int flags;       //AI_PASSIVE, AI_CANONNAME, etc
    int family;      //AF_INET, AF_INET6, AF_UNSPEC
    int socktype;    //SOCK_STREAM, SOCK_DGRA
    int protocol;    //use 0 for "any"
    size_t addrlen;  //size of addr in bytes
    sockaddr* addr;  //_in or _in6
    char* canonname; //full canonical hostname
};

int main() {
    sockaddr_in4 sa;
    sockaddr_in6 sa6;
    inet_pton(AF_INET,"10.12.110.57",  &sa.addr);
    inet_pton(AF_INET6, "201:db8:63b3:1::3490", &sa6.addr);
    return 0;
}
