//+private file
package main

import "core:net"
import "core:os"
import "core:fmt"

PORT :: 4950
MAX_BUF_LEN :: 255

main :: proc() {
    udp_sock :net.UDP_Socket
    net_err :net.Network_Error
    udp_sock, net_err = net.make_bound_udp_socket(net.IP4_Any, PORT)
    if net_err != nil {
        fmt.eprintfln("listener ERROR %q: net make udp socket")
        os.exit(1)
    }
    defer net.close(udp_sock)

    fmt.println("listener: waiting to recv udp...")

    rec_buf :[MAX_BUF_LEN]byte

    bytes :int
    rempoint :net.Endpoint
    bytes, rempoint, net_err = net.recv_udp(udp_sock, rec_buf[:])

    if net_err != nil {
        fmt.eprintfln("listener ERROR %q: net recv udp")
        os.exit(1)
    }

    fmt.println("listener: got packet from", rempoint.address)
    fmt.printfln("listener: packet is %d bytes long", bytes)
    fmt.printfln("listener: packet contains '%s'", string(rec_buf[:bytes]))
}
