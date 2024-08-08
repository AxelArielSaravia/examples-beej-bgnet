package showip

import "core:net"
import "core:os"
import "core:fmt"

Records :: struct {
    ip4 :[]net.DNS_Record,
    ip6 :[]net.DNS_Record,
}

get_dns_records :: proc(
    hostname: string,
    allocator := context.allocator,
) -> (rec :Records, err :net.DNS_Error) {
    rec_ip4 :[]net.DNS_Record
    rec_ip6 :[]net.DNS_Record
    rec_ip4, err = net.get_dns_records_from_os(hostname, .IP4, allocator)
    if err != nil {
        return
    }
    rec_ip6, err = net.get_dns_records_from_os(hostname, .IP6, allocator)
    if err != nil {
        return
    }
    return Records{rec_ip4, rec_ip6}, nil
}

destroy_records :: proc(records :Records) {
    net.destroy_dns_records(records.ip4)
    net.destroy_dns_records(records.ip6)
}

main :: proc() {
    if len(os.args) != 2 {
        fmt.println("Usage:", os.args[0], "hostname")
        os.exit(1)
    }

    hostname :string = os.args[1]
    records, dns_err := get_dns_records(hostname)
    if dns_err == .Invalid_Hostname_Error {
        fmt.eprintln("ERROR: hostane is invalid")
        os.exit(1)
    }
    if dns_err != nil {
        fmt.eprintfln("ERROR %d: dns resolve error", dns_err)
        os.exit(1)
    }
    defer destroy_records(records)

    if len(records.ip4) > 0 {
        fmt.printfln("IP4:")
        for rec in records.ip4 {
            rec := rec.(net.DNS_Record_IP4)
            fmt.println("\tname:    ", rec.record_name)
            fmt.println("\taddress: ", net.address_to_string(rec.address))
            fmt.println("\t---")
        }
    }
    if len(records.ip6) > 0 {
        fmt.printfln("IP6:")
        for rec in records.ip6 {
            rec := rec.(net.DNS_Record_IP6)
            fmt.println("\tname:    ", rec.record_name)
            fmt.println("\taddress: ", net.address_to_string(rec.address))
            fmt.println("\t---")
        }
    }
}
