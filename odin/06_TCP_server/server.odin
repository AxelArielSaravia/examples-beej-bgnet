//+private file
package main

import "core:net"
import "core:os"
import "core:fmt"
import "core:sys/linux"

PORT :: 3490
BACKLOG :: 10

main :: proc() {
    tcp_socket, net_err := net.listen_tcp(
        {address = net.IP4_Any, port = PORT},
        BACKLOG,
    )
    if net_err != nil {
        fmt.eprintfln("server ERROR %q: net listen tcp", net_err)
        os.exit(1)
    }

    defer net.close(tcp_socket)

    msg := "Hello, world"

    fmt.println("server: waiting fo connections...")

    for {
        client_sock, source, net_err := net.accept_tcp(tcp_socket)
        if net_err != nil {
            fmt.eprintfln("server ERROR %q: net accept tcp", net_err)
            os.exit(1)
        }
        defer net.close(client_sock)

        pid, lerrno := linux.fork()
        if lerrno != .NONE {
            fmt.eprintfln("server ERROR %q: linux fork", lerrno)
            os.exit(1)
        }

        if pid == 0 {
            fmt.println("server: got connection from", source.address)
            _, net_err = net.send_tcp(client_sock, transmute([]byte)msg)
            if net_err != nil {
                fmt.eprintfln("server ERROR %q: net send tcp", net_err)
                os.exit(1)
            }
            os.exit(0)
        } else {
            _, lerrno = linux.wait4(pid, nil, nil, nil)
            if lerrno != .NONE {
                fmt.eprintfln("server ERROR %q: linux wait4", lerrno)
                os.exit(1)
            }
        }
    }
}
