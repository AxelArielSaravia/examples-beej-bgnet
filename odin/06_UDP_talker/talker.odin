//+private file
package main

import "core:net"
import "core:os"
import "core:fmt"

PORT :: 4950

main :: proc() {
    if len(os.args) != 3 {
        fmt.eprintln("Usage: talker <hostname> <message>")
        os.exit(1)
    }
    hostname :string = os.args[1]
    msg :string = os.args[2]

    net_err :net.Network_Error

    ep4 :net.Endpoint
    ep4, net_err = net.resolve_ip4(hostname)
    if net_err == .Invalid_Hostname_Error {
        fmt.eprintfln("talker ERROR: invalid hostname")
        os.exit(1)
    }
    if net_err != nil {
        fmt.eprintfln("talker ERROR %v: net resolve_ip4", net_err)
        os.exit(1)
    }
    ep4.port = PORT

    os_socket :net.UDP_Socket
    os_socket, net_err = net.make_unbound_udp_socket(.IP4)
    if net_err != nil {
        fmt.eprintfln("talker ERROR %v: net socket", net_err)
        os.exit(1)
    }
    defer net.close(os_socket)

    bytes :int
    bytes, net_err = net.send_udp(os_socket, transmute([]byte)msg, ep4)
    if net_err != nil {
        fmt.eprintfln("talker ERROR %d %v: net send_udp", net_err, net_err)
        os.exit(1)
    }
    fmt.printfln("talker: send %d bytes to %s", bytes, hostname)
}
