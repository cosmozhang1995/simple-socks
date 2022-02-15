#ifndef _SS_NETWORK_SS_INET_H_
#define _SS_NETWORK_SS_INET_H_

#include <netinet/in.h>

typedef union {
    sa_family_t          domain;
    struct sockaddr_in   ipv4;
    struct sockaddr_in6  ipv6;
} ss_addr_t;

ss_addr_t ss_make_ipv4_addr(const char *addrstr, in_port_t port);

#endif // _SS_NETWORK_SS_INET_H_
