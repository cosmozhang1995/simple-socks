#ifndef _SS_SERVICE_SS_DNS_H_
#define _SS_SERVICE_SS_DNS_H_

#include "util/ss_strmap_def.h"
#include "util/ss_ring_buffer.h"
#include "network/ss_inet.h"

#define SS_DOMAIN_NAME_MAX_LENGTH 255
#define SS_DNML SS_DOMAIN_NAME_MAX_LENGTH

typedef struct ss_dns_service_s ss_dns_service_t;

struct ss_dns_service_s {
    ss_strmap_t       map;
    ss_bool_t         dead;
    ss_ring_buffer_t  recv_buffer;
    ss_ring_buffer_t  send_buffer;
};

ss_dns_service_t *ss_dns_service_start(ss_addr_t addr);
void ss_dns_service_stop(ss_dns_service_t *);

#endif // _SS_SERVICE_SS_DNS_H_
