package main

import "core:net"
import "core:fmt"
import "core:os"

main :: proc() {
    err :net.Network_Error

    socket :net.Any_Socket
    socket, err = net.create_socket(.IP4, .UDP)
    if err != nil {
        fmt.printfln("ERROR %q: net.create_socket", err)
        os.exit(1)
    }

    endpoint := net.Endpoint{
        address = net.IP4_Any,
        port = 3490,
    }

    err = net.bind(socket, endpoint);
    if err != nil {
        fmt.printfln("ERROR %q: net.bind", err)
        os.exit(1)
    }
    fmt.println("Endpoint:", endpoint)
    fmt.println("Socket:", socket)
}
