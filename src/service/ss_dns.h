#ifndef _SS_SERVICE_SS_DNS_H_
#define _SS_SERVICE_SS_DNS_H_

#include "common/ss_time.h"
#include "util/ss_io_helper.h"
#include "util/ss_strmap_def.h"
#include "util/ss_ring_buffer.h"
#include "network/ss_inet.h"

#define SS_DOMAIN_NAME_MAX_LENGTH 254
#define SS_DNML SS_DOMAIN_NAME_MAX_LENGTH

typedef struct ss_dns_service_s ss_dns_service_t;

struct ss_dns_service_s {
    ss_time_t         min_request_interval;
};

ss_dns_service_t *ss_dns_service_start(ss_addr_t addr);
void ss_dns_service_stop(ss_dns_service_t *);

ss_io_err_t ss_dns_fetch(ss_dns_service_t *service, const char *domain_name, ss_addr_t *addr);
ss_io_err_t ss_dns_resolve_start(ss_dns_service_t *service, const char *domain_name);
ss_bool_t ss_dns_get(ss_dns_service_t *service, const char *domain_name, ss_addr_t *addr);

#endif // _SS_SERVICE_SS_DNS_H_
