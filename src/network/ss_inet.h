#ifndef _SS_NETWORK_SS_INET_H_
#define _SS_NETWORK_SS_INET_H_

#include <netinet/in.h>

#include "common/ss_types.h"

#define SS_IPV4_ADDR_SIZE  4
#define SS_IPV6_ADDR_SIZE 16

union ss_addr_u {
    sa_family_t          domain;
    struct sockaddr_in   ipv4;
    struct sockaddr_in6  ipv6;
};

typedef union ss_addr_u ss_addr_t;

ss_addr_t ss_make_ipv4_addr(const char *addrstr, in_port_t port);

ss_bool_t ss_addr_equal(ss_addr_t, ss_addr_t);

int ss_addr_format(char *dest, const char *format, ss_addr_t addr);

#endif // _SS_NETWORK_SS_INET_H_
