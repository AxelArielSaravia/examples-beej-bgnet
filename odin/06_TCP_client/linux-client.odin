//+private file
package main

import "core:fmt"
import "core:net"
import "core:os"
import "core:sys/linux"

PORT :: 3490
MAX_DATA_SIZE :: 255

get_ip4_endpoint :: proc(
    hostname :string,
    port :int,
) -> (ep4 :net.Endpoint, err :net.Network_Error) {
    ep4, err = net.resolve_ip4(hostname)
    if err != nil {
        return
    }
    ep4.port = PORT
    return
}

main :: proc() {
    if len(os.args) != 2 {
        fmt.eprintln("Usage: client <hostname>")
        os.exit(1)
    }

    hostname :string = os.args[1]

    ep4, net_err := get_ip4_endpoint(hostname, PORT)
    if net_err == .Invalid_Hostname_Error {
        fmt.eprintfln("client ERROR: invalid hostname")
        os.exit(1)
    }
    if net_err != nil {
        fmt.eprintfln("client ERROR %q: net get dns records", net_err)
        os.exit(1)
    }

    os_sock :linux.Fd
    errno :linux.Errno
    os_sock, errno = linux.socket(.INET, .STREAM, {}, .TCP)
    if errno != .NONE {
        fmt.eprintfln("client ERROR %q: linux socket", errno)
        os.exit(1)
    }
    defer linux.close(os_sock)

    addr := linux.Sock_Addr_Any{
        ipv4 = {
            sin_family = .INET,
            sin_port = u16be(ep4.port),
            sin_addr = ([4]u8)(ep4.address.(net.IP4_Address)),
        }
    }

    errno = linux.connect(os_sock, &addr)
    if errno != .NONE {
        fmt.eprintfln("client ERROR %q: linux connect", errno)
        os.exit(1)
    }

    fmt.println("client: connection to", hostname)

    buf :[MAX_DATA_SIZE]byte
    bytes_read :int
    bytes_read, errno = linux.recv(os_sock, buf[:], {})
    if errno != .NONE {
        fmt.eprintfln("client ERROR %q: linux recv", errno)
        os.exit(1)
    }
    fmt.printfln("client: received '%s'", string(buf[:bytes_read]))
}
