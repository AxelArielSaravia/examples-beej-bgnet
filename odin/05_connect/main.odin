package main

import "core:net"
import "core:os"
import "core:fmt"


HOSTNAME :: "www.example.com"
PORT :: ":3490"

main :: proc() {
    socket :net.TCP_Socket
    err :net.Network_Error
    socket, err = net.dial_tcp_from_hostname_and_port_string(HOSTNAME+PORT)

    if err != nil {
        fmt.eprintfln("ERROR %q: dial tcp", err)
        os.exit(1)
    }
    fmt.println("Connected")
}
