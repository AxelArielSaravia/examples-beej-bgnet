//+private file
package main

import "core:sys/linux"
import "core:os"
import "core:fmt"
import "core:net"

PORT :: 4950
MAX_BUF_LEN :: 255

main :: proc() {
    os_socket, lerrno := linux.socket(.INET, .DGRAM, {}, .UDP)
    if lerrno != .NONE {
        fmt.eprintfln("listener ERROR %q: linux socket")
        os.exit(1)
    }
    defer linux.close(os_socket)

    lerrno = linux.bind(
        os_socket,
        &linux.Sock_Addr_Any{
            ipv4 = {
                sin_family = .INET,
                sin_port = u16be(PORT),
                sin_addr = ([4]u8)(net.IP4_Any)
            },
        },
    )
    if lerrno != .NONE {
        fmt.eprintfln("listener ERROR %q: linux bind")
        os.exit(1)
    }

    fmt.println("listener: waiting to recvfrom...")

    rec_buf :[MAX_BUF_LEN]byte

    addr :linux.Sock_Addr_Any
    bytes :int
    bytes, lerrno = linux.recvfrom(os_socket, rec_buf[:], {}, &addr)
    if lerrno != .NONE {
        fmt.eprintfln("listener ERROR %q: linux recvfrom")
        os.exit(1)
    }

    fmt.println("listener: got packet from", addr.ipv4.sin_addr)
    fmt.printfln("listener: packet is %d bytes long", bytes)
    fmt.printfln("listener: packet contains '%s'", string(rec_buf[:]))
}
