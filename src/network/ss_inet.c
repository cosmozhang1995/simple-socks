#include "ss_inet.h"

#include <arpa/inet.h>

ss_addr_t ss_make_ipv4_addr(const char *addrstr, in_port_t port)
{
    ss_addr_t addr;
    addr.ipv4.sin_family = AF_INET;
    addr.ipv4.sin_port = htons(port);
    addr.ipv4.sin_addr.s_addr = inet_addr(addrstr);
    return addr;
}