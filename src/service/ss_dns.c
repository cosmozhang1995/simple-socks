#include "service/ss_dns.h"

#include <stdlib.h>
#include <memory.h>

#include "common/ss_defs.h"
#include "util/ss_strmap.h"
#include "util/ss_io_helper.h"
#include "network/ss_connection.h"

static void ss_dns_recv_handler(ss_connection_t *connection);
static void ss_dns_send_handler(ss_connection_t *connection);
static void ss_dns_destroy_handler(ss_connection_t *connection);
static ss_bool_t ss_dns_check_status(ss_connection_t *connection);

#define GET_CONTEXT(connection) ((ss_dns_service_t *)connection->context)

void ss_dns_service_initialize(ss_dns_service_t *service)
{
    ss_strmap_initialize(&service->map);
    service->dead = SS_FALSE;
}

ss_bool_t ss_dns_service_connect(ss_dns_service_t *service, ss_addr_t addr)
{
    ss_connection_t *connection;
    connection = ss_network_connect(SOCK_DGRAM, addr);
    if (!connection) return SS_FALSE;
    connection->recv_handler = (ss_connection_recv_handler_t)ss_dns_recv_handler;
    connection->send_handler = (ss_connection_send_handler_t)ss_dns_send_handler;
    connection->destroy_handler = (ss_connection_destroy_handler_t)ss_dns_destroy_handler;
    connection->check_status = (ss_connection_status_function_t)ss_dns_check_status;
    connection->context = service;
    return SS_TRUE;
}

ss_dns_service_t *ss_dns_service_create(ss_addr_t addr)
{
    ss_dns_service_t *service;
    service = malloc(sizeof(ss_dns_service_t));
    ss_dns_service_initialize(service);
    if (!ss_dns_service_connect(service, addr)) {
        ss_dns_service_release(service);
        return SS_NULL;
    }
    return service;
}

void ss_dns_service_uninitialize(ss_dns_service_t *service)
{
    ss_strmap_uninitialize(&service->map);
}

void ss_dns_service_disconnect(ss_dns_service_t *service)
{
    service->dead = SS_TRUE;
}

void ss_dns_service_release(ss_dns_service_t *service)
{
    ss_dns_service_uninitialize(service);
    free(service);
}

ss_dns_service_t *ss_dns_service_start(ss_addr_t addr)
{
}

void ss_dns_service_stop(ss_dns_service_t *service)
{
    ss_dns_service_disconnect(service);
}

typedef struct ss_dns_header_s ss_dns_header_t;

typedef struct ss_dns_rr_s ss_dns_rr_t;

struct ss_dns_header_s {
    ss_uint16_t   tid;
    ss_uint16_t   flags;
    ss_uint16_t   nqueries;
    ss_uint16_t   nanswers;
    ss_uint16_t   nauthorities;
    ss_uint16_t   nadditionals;
};

#define SS_DNS_

static ss_io_err_t recv_query(int fd, ss_dns_service_t *service, size_t *offset);
static ss_io_err_t recv_answer(int fd, ss_dns_service_t *service, size_t *offset);
static ss_io_err_t recv_authority(int fd, ss_dns_service_t *service, size_t *offset);
static ss_io_err_t recv_additional(int fd, ss_dns_service_t *service, size_t *offset);

static void ss_dns_recv_handler(ss_connection_t *connection)
{
    ss_dns_header_t     header;
    ss_dns_service_t   *service;
    ss_uint16_t         i;
    size_t              offset;
    ss_io_err_t         rc;

    rc = SS_IO_OK;
    offset = 0;
    service = GET_CONTEXT(connection);
    if ((rc = ss_recv_via_buffer_auto_inc(&header, connection->fd, &service->recv_buffer, &offset, sizeof(header))) != SS_IO_OK) {
        return rc;
    }
    for (i = 0; i < header.nqueries; i++) {
        if ((rc = recv_query(connection->fd, service, &offset)) != SS_IO_OK) goto _l_error;
    }
    for (i = 0; i < header.nanswers; i++) {
        if ((rc = recv_answer(connection->fd, service, &offset)) != SS_IO_OK) goto _l_error;
    }
    for (i = 0; i < header.nauthorities; i++) {
        if ((rc = recv_authority(connection->fd, service, &offset)) != SS_IO_OK) goto _l_error;
    }
    for (i = 0; i < header.nadditionals; i++) {
        if ((rc = recv_additional(connection->fd, service, &offset)) != SS_IO_OK) goto _l_error;
    }
_l_success:
    ss_ring_buffer_read(SS_NULL, &service->recv_buffer, offset);
    return;
_l_error:
    if (rc < 0) service->dead = SS_FALSE;
    return;
}

static ss_io_err_t recv_domain_name(int fd, ss_dns_service_t *service, size_t *offset, char *domain)
{
    ss_uint8_t   label_length;
    ss_uint16_t  total_length;
    ss_io_err_t  rc;

    rc = SS_IO_OK;
    total_length = 0;
    while (SS_TRUE) {
        if ((rc = ss_recv_via_buffer_auto_inc(&label_length, fd, &service->recv_buffer, offset, sizeof(label_length))) != SS_IO_OK) {
            goto _l_end;
        }
        if (label_length == 0) {
            rc = SS_IO_OK;
            goto _l_end;
        }
        if (label_length & 0xC0) {
            printf("ERROR: response query need compression\n");
            rc = SS_IO_ERROR;
            goto _l_end;
        }
        if (total_length + (ss_uint16_t)label_length > SS_DNML) {
            printf("ERROR: domain name exceeds length restriction\n");
            rc = SS_IO_ERROR;
            goto _l_end;
        }
        if ((rc = ss_recv_via_buffer_auto_inc(
                (void *)(domain ? domain + total_length : SS_NULL),
                fd, &service->recv_buffer, offset, (size_t)label_length)
            ) != SS_IO_OK) {
            goto _l_end;
        }
        total_length += (ss_uint16_t)label_length;
        if (domain) domain[total_length++] = '.';
    }
_l_end:
    if (domain) {
        if (total_length != 0) {
            domain[total_length - 1] = '\0';
        } else {
            domain[0] = '\0';
        }
    }
    return rc;
}

static ss_io_err_t recv_query(int fd, ss_dns_service_t *service, size_t *offset)
{
    int          rc;
    // receive QNAME
    if ((rc = recv_domain_name(fd, service, offset, SS_NULL)) != SS_IO_OK) {
        return rc;
    }
    // receive QTYPE
    if ((rc = ss_recv_via_buffer_auto_inc(SS_NULL, fd, &service->recv_buffer, offset, sizeof(ss_uint16_t))) != SS_IO_OK) {
        return rc;
    }
    // receive QCLASS
    if ((rc = ss_recv_via_buffer_auto_inc(SS_NULL, fd, &service->recv_buffer, offset, sizeof(ss_uint16_t))) != SS_IO_OK) {
        return rc;
    }
    return SS_IO_OK;
}

#define SS_RR_TYPE_A                1  // a host address
#define SS_RR_TYPE_NS               2  // an authoritative name server
#define SS_RR_TYPE_MD               3  // a mail destination (Obsolete - use MX)
#define SS_RR_TYPE_MF               4  // a mail forwarder (Obsolete - use MX)
#define SS_RR_TYPE_CNAME            5  // the canonical name for an alias
#define SS_RR_TYPE_SOA              6  // marks the start of a zone of authority
#define SS_RR_TYPE_MB               7  // a mailbox domain name (EXPERIMENTAL)
#define SS_RR_TYPE_MG               8  // a mail group member (EXPERIMENTAL)
#define SS_RR_TYPE_MR               9  // a mail rename domain name (EXPERIMENTAL)
#define SS_RR_TYPE_NULL            10  // a null RR (EXPERIMENTAL)
#define SS_RR_TYPE_WKS             11  // a well known service description
#define SS_RR_TYPE_PTR             12  // a domain name pointer
#define SS_RR_TYPE_HINFO           13  // host information
#define SS_RR_TYPE_MINFO           14  // mailbox or mail list information
#define SS_RR_TYPE_MX              15  // mail exchange
#define SS_RR_TYPE_TXT             16  // text strings

#define SS_RR_CLASS_IN              1  // the Internet
#define SS_RR_CLASS_CS              2  // the CSNET class
#define SS_RR_CLASS_CH              3  // the CHAOS class
#define SS_RR_CLASS_HS              4  // Hesiod [Dyer 87]

static ss_inline
const char *translate_rr_class(ss_uint16_t cls)
{
    switch (cls) {
    case SS_RR_CLASS_IN:
        return "IN";
    case SS_RR_CLASS_CS:
        return "CS";
    case SS_RR_CLASS_CH:
        return "CH";
    case SS_RR_CLASS_HS:
        return "HS";
    default:
        break;
    }
    return "<unknown>";
}

typedef struct ss_dns_rr_meta_s ss_dns_rr_meta_t;

struct ss_dns_rr_meta_s {
    ss_uint16_t  type;
    ss_uint16_t  class;
    ss_uint16_t  ttl;
    ss_uint16_t  rdlength;
};

struct ss_dns_rr_s {
    char               *name;
    ss_dns_rr_meta_t    meta;
    void               *rdata;
};

static void ss_dns_rr_initialize(ss_dns_rr_t *rr)
{
    memset((void *)rr, 0, sizeof(rr));
    rr->name = malloc(SS_DNML);
}

static void ss_dns_rr_uninitialize(ss_dns_rr_t *rr)
{
    if (rr->name) free(rr->name);
    if (rr->rdata) free(rr->rdata);
}

static ss_io_err_t recv_rdata_a(int fd, ss_dns_service_t *service, size_t *offset, ss_dns_rr_meta_t meta, void *data);
static ss_io_err_t recv_rdata_any(int fd, ss_dns_service_t *service, size_t *offset, ss_dns_rr_meta_t meta, void *data);

static ss_io_err_t recv_rr(int fd, ss_dns_service_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    int          rc;
    // receive NAME
    if ((rc = recv_domain_name(fd, service, offset, rr ? rr->name : SS_NULL)) != SS_IO_OK) {
        return rc;
    }
    // receive meta
    if ((rc = ss_recv_via_buffer_auto_inc(rr ? &rr->meta : SS_NULL, fd, &service->recv_buffer, offset, sizeof(rr->meta))) != SS_IO_OK) {
        return 1;
    }
    if (rr->meta.class != SS_RR_CLASS_IN) {
        printf("ERROR: Unsupported RR class %s\n", translate_rr_class(rr->meta.class));
    }
    // receive RDATA
    // if (!ss_recv_via_buffer_auto_inc(&rr->meta, fd, &service->recv_buffer, offset, sizeof(rr->meta))) {
    //     return 1;
    // }
}

static ss_io_err_t recv_rdata_a(int fd, ss_dns_service_t *service, size_t *offset, ss_dns_rr_meta_t meta, void *data)
{
    if (meta.rdlength)
}

static ss_io_err_t recv_answer(int fd, ss_dns_service_t *service, size_t *offset)
{
    int          rc;
    // receive NAME
    if ((rc = recv_domain_name(fd, service, offset, SS_NULL)) != 0) {
        return rc;
    }
}

static ss_io_err_t recv_authority(int fd, ss_dns_service_t *service, size_t *offset)
{
}

static ss_int8_t recv_additional(int fd, ss_dns_service_t *service, size_t *offset)
{
}

static void ss_dns_send_handler(ss_connection_t *connection)
{
    ss_ring_buffer_send(connection->fd,
        &GET_CONTEXT(connection)->send_buffer,
        GET_CONTEXT(connection)->send_buffer.length);
}

static void ss_dns_destroy_handler(ss_connection_t *connection)
{
}

static ss_bool_t ss_dns_check_status(ss_connection_t *connection)
{
    ss_dns_service_t   *service;
    service = GET_CONTEXT(connection);
    return !service->dead;
}

