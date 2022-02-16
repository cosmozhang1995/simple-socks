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

ss_bool_t ss_addr_equal(ss_addr_t a1, ss_addr_t a2)
{
    if (a1.domain != a2.domain) return SS_FALSE;
    switch (a1.domain) {
    case AF_INET:
        if (a1.ipv4.sin_addr.s_addr != a2.ipv4.sin_addr.s_addr)
            return SS_FALSE;
        if (a1.ipv4.sin_port != a2.ipv4.sin_port)
            return SS_FALSE;
        return SS_TRUE;
    case AF_INET6:
        if (((ss_uint32_t *)&a1.ipv6.sin6_addr)[0] != ((ss_uint32_t *)&a2.ipv6.sin6_addr)[0])
            return SS_FALSE;
        if (((ss_uint32_t *)&a1.ipv6.sin6_addr)[1] != ((ss_uint32_t *)&a2.ipv6.sin6_addr)[1])
            return SS_FALSE;
        if (((ss_uint32_t *)&a1.ipv6.sin6_addr)[2] != ((ss_uint32_t *)&a2.ipv6.sin6_addr)[2])
            return SS_FALSE;
        if (((ss_uint32_t *)&a1.ipv6.sin6_addr)[3] != ((ss_uint32_t *)&a2.ipv6.sin6_addr)[3])
            return SS_FALSE;
        if (a1.ipv6.sin6_port != a2.ipv6.sin6_port)
            return SS_FALSE;
        return SS_TRUE;
    }
}
