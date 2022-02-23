#include "ss_inet.h"

#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

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

static int ss_addr_stringify_ipv4_address(char *dest, ss_addr_t addr);
static int ss_addr_stringify_ipv4_port(char *dest, ss_addr_t addr);
static int ss_addr_stringify_ipv6_address(char *dest, ss_addr_t addr);
static int ss_addr_stringify_ipv6_port(char *dest, ss_addr_t addr);

int ss_addr_format(char *dest, const char *format, ss_addr_t addr)
{
    int          i, j;
    ss_bool_t    ff;
    char         ch;
    int          rc;

    dest[0] = 0;
    ff = SS_FALSE;
    for (i = 0, j = 0; (ch = format[i]) != 0; i++) {
        if (ff) {
            rc = 0;
            switch (ch) {
                case 'a':
                    switch (addr.domain) {
                    case AF_INET:
                        rc = ss_addr_stringify_ipv4_address(dest + j, addr);
                        break;
                    case AF_INET6:
                        rc = ss_addr_stringify_ipv6_address(dest + j, addr);
                        break;
                    default:
                        break;
                    }
                    break;
                case 'p':
                    switch (addr.domain) {
                    case AF_INET:
                        rc = ss_addr_stringify_ipv4_port(dest + j, addr);
                        break;
                    case AF_INET6:
                        rc = ss_addr_stringify_ipv6_port(dest + j, addr);
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
            }
            if (rc == 0) {
                dest[j] = '%';
                dest[j + 1] = ch;
                rc = 2;
            }
            if (rc < 0) {
                goto _l_end;
            }
            j += rc;
        } else if (ch == '%') {
            ff = SS_TRUE;
        } else {
            dest[j++] = ch;
        }
    }
_l_end:
    dest[j] = 0;
    if (rc < 0) return rc;
    return j;
}

int ss_addr_stringify_ipv4_address(char *dest, ss_addr_t addr)
{
    return sprintf(dest, "%d.%d.%d.%d",
        (int)(((ss_uint8_t *)&addr.ipv4.sin_addr)[0]),
        (int)(((ss_uint8_t *)&addr.ipv4.sin_addr)[1]),
        (int)(((ss_uint8_t *)&addr.ipv4.sin_addr)[2]),
        (int)(((ss_uint8_t *)&addr.ipv4.sin_addr)[3])
    );
}

int ss_addr_stringify_ipv4_port(char *dest, ss_addr_t addr)
{
    return sprintf(dest, "%d", (int)addr.ipv4.sin_port);
}

int ss_addr_stringify_ipv6_address(char *dest, ss_addr_t addr)
{
    return sprintf(dest, "%x:%x:%x:%x:%x:%x:%x:%x",
        ntohs(((ss_uint16_t *)&addr.ipv6.sin6_addr)[0]),
        ntohs(((ss_uint16_t *)&addr.ipv6.sin6_addr)[1]),
        ntohs(((ss_uint16_t *)&addr.ipv6.sin6_addr)[2]),
        ntohs(((ss_uint16_t *)&addr.ipv6.sin6_addr)[3]),
        ntohs(((ss_uint16_t *)&addr.ipv6.sin6_addr)[4]),
        ntohs(((ss_uint16_t *)&addr.ipv6.sin6_addr)[5]),
        ntohs(((ss_uint16_t *)&addr.ipv6.sin6_addr)[6]),
        ntohs(((ss_uint16_t *)&addr.ipv6.sin6_addr)[7])
    );
}

int ss_addr_stringify_ipv6_port(char *dest, ss_addr_t addr)
{
    return sprintf(dest, "%d", (int)addr.ipv6.sin6_port);
}
