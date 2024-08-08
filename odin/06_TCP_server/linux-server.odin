//+private file
package main

import "core:sys/linux"
import "core:os"
import "core:fmt"
import "core:net"

PORT :: 3490
BACKLOG :: 10

main :: proc() {
    os_socket, lerrno := linux.socket(.INET, .STREAM, {}, .TCP)
    if lerrno != .NONE {
        fmt.eprintfln("server ERROR %q: linux socket", lerrno)
        os.exit(1)
    }
    defer linux.close(os_socket)

    reuse_addr :b32 = true
    lerrno = linux.setsockopt_sock(os_socket, .SOCKET, .REUSEADDR, &reuse_addr)
    if lerrno != .NONE {
        fmt.eprintfln("server ERROR %q: linux setsockopt", lerrno)
        os.exit(1)
    }

    addr := linux.Sock_Addr_Any{
        ipv4 = {
            sin_family = .INET,
            sin_port = u16be(PORT),
            sin_addr =([4]u8)(net.IP4_Any)
        }
    }

    lerrno = linux.bind(os_socket, &addr)
    if lerrno != .NONE {
        fmt.eprintfln("server ERROR %q: linux bind", lerrno)
        os.exit(1)
    }

    lerrno = linux.listen(os_socket, BACKLOG)
    if lerrno != .NONE {
        fmt.eprintfln("server ERROR %q: linux listen", lerrno)
        os.exit(1)
    }

    fmt.println("server: waiting fo connections...")

    msg := "Hello, world"
    for {
        addr := linux.Sock_Addr_Any{}
        client_sock :linux.Fd
        client_sock, lerrno = linux.accept(os_socket, &addr)
        if lerrno != .NONE {
            fmt.eprintfln("server ERROR %q: linux accept", lerrno)
            os.exit(1)
        }
        val: b32 = cast(b32)net.default_tcp_options.no_delay
        _ = linux.setsockopt_tcp(client_sock, .TCP, .NODELAY, &val)

        defer linux.close(client_sock)

        pid :linux.Pid
        pid, lerrno = linux.fork()
        if lerrno != .NONE {
            fmt.eprintfln("server ERROR %q: linux fork", lerrno)
            os.exit(1)
        }


        if pid == 0 {
            fmt.printfln("server: got connection from", addr.ipv4.sin_addr)

            _, lerrno = linux.send(client_sock, transmute([]byte)msg, {.NOSIGNAL})
            if lerrno != .NONE {
                fmt.eprintfln("server ERROR %q: linux send", lerrno)
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
