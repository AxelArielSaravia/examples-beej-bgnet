//+private file
package main

import "core:fmt"
import "core:net"
import "core:os"

PORT :: 3490
MAX_DATA_SIZE :: 255

main :: proc() {
    if len(os.args) != 2 {
        fmt.eprintln("Usage: client <hostname>")
        os.exit(1)
    }
    hostname :string = os.args[1]

    tcp_socket, net_err := net.dial_tcp_from_hostname_with_port_override(
        hostname,
        PORT,
    )
    if net_err != nil {
        fmt.eprintfln("client ERROR %q: tcp dial", net_err)
        os.exit(1)
    }
    defer net.close(tcp_socket)

    fmt.println("clien: connect to", hostname)

    buf :[MAX_DATA_SIZE]byte

    bytes_read :int
    bytes_read, net_err = net.recv_tcp(tcp_socket, buf[:])
    if net_err != nil {
        fmt.eprintfln("client ERROR %q: tcp recv", net_err)
        os.exit(1)
    }
    fmt.printfln("client: reveived '%s'", string(buf[:bytes_read]))
}
