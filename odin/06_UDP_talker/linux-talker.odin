//+private file
package main

import "core:sys/linux"
import "core:os"
import "core:fmt"
import "core:net"

PORT :: 4950

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
    if len(os.args) != 3 {
        fmt.eprintln("Usage: talker <hostname> <message>")
        os.exit(1)
    }
    hostname :string = os.args[1]
    msg :string = os.args[2]

    ep4, net_err := get_ip4_endpoint(hostname, PORT)
    if net_err == .Invalid_Hostname_Error {
        fmt.eprintfln("talker ERROR: invalid hostname")
        os.exit(1)
    }
    if net_err != nil {
        fmt.eprintfln("talker ERROR %q: net get dns records", net_err)
        os.exit(1)
    }
    os_socket, lerrno := linux.socket(.INET, .DGRAM, {}, .UDP)
    if lerrno != .NONE {
        fmt.eprintfln("talker ERROR %q: linux socket", lerrno)
        os.exit(1)
    }
    defer linux.close(os_socket)

    bytes :int
    bytes, lerrno = linux.sendto(
        sock=os_socket,
        buf=transmute([]byte)msg,
        flags={},
        addr=&linux.Sock_Addr_Any{
            ipv4 = {
                sin_family = .INET,
                sin_port = u16be(ep4.port),
                sin_addr = ([4]u8)(ep4.address.(net.IP4_Address)),
            },
        },
    )
    if lerrno != .NONE {
        fmt.eprintfln("talker ERROR %q: linux sendto", lerrno)
        os.exit(1)
    }
    fmt.printfln("talker: send %d bytes to %s", bytes, hostname)
}
