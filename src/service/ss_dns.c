#include "service/ss_dns.h"

#include <stdlib.h>
#include <memory.h>
#include <time.h>

#include "common/ss_defs.h"
#include "util/ss_strmap.h"
#include "util/ss_io_helper.h"
#include "util/ss_linked_list.h"
#include "network/ss_connection.h"

#define SS_TYPE_A                1  // a host address
#define SS_TYPE_NS               2  // an authoritative name server
#define SS_TYPE_MD               3  // a mail destination (Obsolete - use MX)
#define SS_TYPE_MF               4  // a mail forwarder (Obsolete - use MX)
#define SS_TYPE_CNAME            5  // the canonical name for an alias
#define SS_TYPE_SOA              6  // marks the start of a zone of authority
#define SS_TYPE_MB               7  // a mailbox domain name (EXPERIMENTAL)
#define SS_TYPE_MG               8  // a mail group member (EXPERIMENTAL)
#define SS_TYPE_MR               9  // a mail rename domain name (EXPERIMENTAL)
#define SS_TYPE_NULL            10  // a null RR (EXPERIMENTAL)
#define SS_TYPE_WKS             11  // a well known service description
#define SS_TYPE_PTR             12  // a domain name pointer
#define SS_TYPE_HINFO           13  // host information
#define SS_TYPE_MINFO           14  // mailbox or mail list information
#define SS_TYPE_MX              15  // mail exchange
#define SS_TYPE_TXT             16  // text strings
#define SS_TYPE_AAAA            28  // ipv6 address

#define SS_CLASS_IN              1  // the Internet
#define SS_CLASS_CS              2  // the CSNET class
#define SS_CLASS_CH              3  // the CHAOS class
#define SS_CLASS_HS              4  // Hesiod [Dyer 87]

typedef struct ss_dns_service_wrapper_s ss_dns_service_wrapper_t;

struct ss_dns_service_wrapper_s {
    ss_dns_service_t  exposed;
    ss_strmap_t       map;
    ss_strmap_t       resolving_map;
    ss_uint16_t       tid_flags[SS_UINT16_MAX >> 4];
    ss_uint16_t       next_tid;
    ss_bool_t         dead;
    ss_ring_buffer_t  recv_buffer;
    ss_ring_buffer_t  send_buffer;
    ss_uint16_t       transaction_id;
};

static void ss_dns_recv_handler(ss_connection_t *connection);
static void ss_dns_send_handler(ss_connection_t *connection);
static void ss_dns_destroy_handler(ss_connection_t *connection);
static ss_bool_t ss_dns_check_status(ss_connection_t *connection);

#define GET_CONTEXT(connection) ((ss_dns_service_wrapper_t *)connection->context)

typedef struct ss_dns_entry_s ss_dns_entry_t;
typedef ss_linked_list_t ss_dns_record_list_t;
typedef struct ss_dns_record_s ss_dns_record_t;

static ss_dns_record_list_t *ss_dns_list_create();
static void ss_dns_list_release(ss_dns_record_list_t *list);

typedef struct ss_dns_header_s ss_dns_header_t;

static void ss_dns_header_query(ss_dns_header_t *header, ss_uint16_t tid, ss_uint16_t nqueries);

static ss_bool_t next_tid(ss_dns_service_wrapper_t *service, ss_uint16_t *tid);
static ss_bool_t allocate_tid(ss_dns_service_wrapper_t *service, ss_uint16_t *tid);
static void occupy_tid(ss_dns_service_wrapper_t *service, ss_uint16_t tid);
static void release_tid(ss_dns_service_wrapper_t *service, ss_uint16_t tid);
static ss_inline ss_uint16_t is_tid_free(ss_uint16_t *flags, ss_uint16_t tid);

static void encode_domain_name(void *dest, const char *dn, size_t dnlen);


static ss_dns_service_wrapper_t *ss_dns_service_start_inner(ss_addr_t addr);
ss_dns_service_t *ss_dns_service_start(ss_addr_t addr)
{
    return (ss_dns_service_t *)ss_dns_service_start(addr);
}

static void ss_dns_service_stop_inner(ss_dns_service_wrapper_t *);
void ss_dns_service_stop(ss_dns_service_t *service)
{
    ss_dns_service_stop_inner((ss_dns_service_wrapper_t *)service);
}

static ss_io_err_t ss_dns_fetch_inner(ss_dns_service_wrapper_t *service, const char *domain_name, ss_addr_t *addr);
ss_io_err_t ss_dns_fetch(ss_dns_service_t *service, const char *domain_name, ss_addr_t *addr)
{
    return ss_dns_fetch_inner((ss_dns_service_wrapper_t *)service, domain_name, addr);
}

static ss_io_err_t ss_dns_resolve_start_inner(ss_dns_service_wrapper_t *service, const char *domain_name);
ss_io_err_t ss_dns_resolve_start(ss_dns_service_t *service, const char *domain_name)
{
    return ss_dns_resolve_start_inner((ss_dns_service_wrapper_t *)service, domain_name);
}

static ss_bool_t ss_dns_get_inner(ss_dns_service_wrapper_t *service, const char *domain_name, ss_addr_t *addr);
ss_bool_t ss_dns_get(ss_dns_service_t *service, const char *domain_name, ss_addr_t *addr)
{
    return ss_dns_get_inner((ss_dns_service_wrapper_t *)service, domain_name, addr);
}


void ss_dns_service_initialize(ss_dns_service_wrapper_t *service)
{
    ss_strmap_initialize(&service->map);
    service->dead = SS_FALSE;
}

ss_bool_t ss_dns_service_connect(ss_dns_service_wrapper_t *service, ss_addr_t addr)
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

ss_dns_service_wrapper_t *ss_dns_service_create(ss_addr_t addr)
{
    ss_dns_service_wrapper_t *service;
    service = malloc(sizeof(ss_dns_service_wrapper_t));
    ss_dns_service_initialize(service);
    if (!ss_dns_service_connect(service, addr)) {
        ss_dns_service_release(service);
        return SS_NULL;
    }
    return service;
}

void ss_dns_service_uninitialize(ss_dns_service_wrapper_t *service)
{
    ss_strmap_uninitialize(&service->map);
}

void ss_dns_service_disconnect(ss_dns_service_wrapper_t *service)
{
    service->dead = SS_TRUE;
}

void ss_dns_service_release(ss_dns_service_wrapper_t *service)
{
    ss_dns_service_uninitialize(service);
    free(service);
}

static
ss_dns_service_wrapper_t *ss_dns_service_start_inner(ss_addr_t addr)
{
}

static
void ss_dns_service_stop_inner(ss_dns_service_wrapper_t *service)
{
    ss_dns_service_disconnect(service);
}

typedef struct ss_dns_question_tail_s ss_dns_question_tail_t;

struct ss_dns_question_tail_s {
    ss_uint16_t qtype;
    ss_uint16_t qclass;
};

/**
 * @brief Fetch a DNS resolved address for the specified domain name. If no local storage, start a new request.
 * 
 * @param service service instance
 * @param dn domain name
 * @param addr fetched address
 * @return ss_io_err_t OK for success. EAGIAN if requesting. Otherwise for failure.
 */
static
ss_io_err_t ss_dns_fetch_inner(ss_dns_service_wrapper_t *service, const char *dn, ss_addr_t *addr)
{
    ss_io_err_t rc;
    if (ss_dns_get(service, dn, addr)) {
        return SS_IO_OK;
    }
    if ((rc = ss_dns_resolve_start(service, dn)) != SS_IO_OK) {
        return rc;
    }
    return SS_IO_EAGAIN;
}

/**
 * @brief Start a DNS query. This will send 2 questions, for IPv4 and IPv6 addresses respectively.
 * 
 * @param service service instance
 * @param dn domain name
 * @return ss_io_err_t 
 */
static
ss_io_err_t ss_dns_resolve_start_inner(ss_dns_service_wrapper_t *service, const char *dn)
{
    ss_variable_t             var;
    ss_uint16_t               tid;
    ss_uint8_t                data[
                                sizeof(ss_dns_header_t)
                                + SS_DNML
                                + sizeof(ss_dns_question_tail_t)
                                + SS_DNML
                                + sizeof(ss_dns_question_tail_t)];
    ss_dns_header_t          *header;
    ss_dns_question_tail_t   *ques_a_tail;
    ss_dns_question_tail_t   *ques_aaaa_tail;
    size_t                    dnlen;
    void                     *ques_a_dn;
    void                     *ques_aaaa_dn;
    size_t                    i;
    ss_uint8_t                labelsz;
    size_t                    reqlen;
    ss_io_err_t               rc;

    if (ss_strmap_get(&service->resolving_map, dn, &var)) {
        return SS_IO_EAGAIN;
    }
    dnlen = strlen(dn);
    if (dnlen > SS_DNML) {
        return SS_IO_ERROR;
    }
    reqlen = 0;
    header = (ss_dns_header_t *)data;
    reqlen += sizeof(ss_dns_header_t);
    ques_a_dn = (void *)(data + reqlen);
    reqlen += dnlen + 2;
    ques_a_tail = (ss_dns_question_tail_t *)(data + reqlen);
    reqlen += sizeof(ss_dns_question_tail_t);
    ques_aaaa_dn = (void *)(data + reqlen);
    reqlen += dnlen + 2;
    ques_aaaa_tail = (ss_dns_question_tail_t *)(data + reqlen);
    reqlen += sizeof(ss_dns_question_tail_t);
    // initialize header data
    if (!next_tid(service, &tid)) {
        return SS_IO_EAGAIN;
    }
    ss_dns_header_query(data, tid, 2);
    // initialize A question QNAME
    encode_domain_name(ques_a_dn, dn, dnlen);
    // initialize A question tail
    ques_a_tail->qtype = SS_TYPE_A;
    ques_a_tail->qclass = SS_CLASS_IN;
    // initialize AAAA question QNAME, which is same to A question QNAME
    memcpy((void *)ques_aaaa_dn, (void *)ques_a_dn, dnlen + 2);
    // initialize AAAA question tail
    ques_aaaa_tail->qtype = SS_TYPE_AAAA;
    ques_aaaa_tail->qclass = SS_CLASS_IN;
    // write question
    if ((rc = ss_send_via_buffer(&service->send_buffer, data, reqlen)) != SS_IO_OK) {
        return SS_IO_ERROR;
    }
    occupy_tid(service, tid);
    return SS_IO_OK;
}

static
ss_bool_t ss_dns_get_inner(ss_dns_service_wrapper_t *service, const char *dn, ss_addr_t *addr)
{
    ss_variable_t var;
    if (ss_strmap_get(&service->map, dn, &var)) {
        *addr = *((ss_addr_t *)var.ptr);
        return SS_TRUE;
    }
    return SS_FALSE;
}

struct ss_dns_entry_s {
    ss_bool_t             requesting;
    ss_dns_record_list_t  list;
};

struct ss_dns_record_s {
    ss_addr_t      addr;
    time_t         expire;
};

static ss_dns_record_t *ss_dns_record_create()
{
    ss_dns_record_t *entry;
    entry = malloc(sizeof(ss_dns_record_t));
    memset(entry, 0, sizeof(ss_dns_record_t));
    return entry;
}

static void ss_dns_record_release(ss_dns_record_t *entry)
{
    free(entry);
}

static void ss_dns_list_initialize(ss_dns_record_list_t *list)
{
    ss_linked_list_initialize(list);
}

static void ss_dns_list_uninitialize(ss_dns_record_list_t *list)
{
    ss_linked_list_uninitialize(list, (ss_linked_list_release_data_function_t)ss_dns_record_release);
}

static ss_dns_entry_t *ss_dns_entry_create()
{
    ss_dns_entry_t *entry;
    entry = malloc(sizeof(ss_dns_entry_t));
    entry->requesting = SS_FALSE;
    ss_dns_list_initialize(&entry->list);
    return entry;
}

static void ss_dns_entry_release(ss_dns_entry_t *entry)
{
    ss_dns_list_uninitialize(&entry->list);
    free(entry);
}

static ss_inline
ss_dns_entry_t *ss_dns_get_entry(ss_dns_service_wrapper_t *service, const char *dn)
{
    ss_variable_t         var;
    if (ss_strmap_get(&service->map, dn, &var)) {
        return (ss_dns_entry_t *)var.ptr;
    }
    return SS_NULL;
}

static ss_inline
ss_dns_entry_t *ss_dns_entry(ss_dns_service_wrapper_t *service, const char *dn)
{
    ss_dns_entry_t        *entry;
    ss_variable_t          var;
    if (ss_strmap_get(&service->map, dn, &var)) {
        return (ss_dns_entry_t *)var.ptr;
    } else {
        entry = ss_dns_entry_create();
        var.ptr = entry;
        ss_strmap_put(&service->map, dn, var, SS_NULL);
    }
    return entry;
}

static void ss_dns_update(ss_dns_service_wrapper_t *service, const char *dn, ss_addr_t addr, ss_int32_t ttl)
{
    ss_dns_record_list_t  *list;
    ss_dns_record_t       *entry, *temp;
    time_t                now;
    ss_bool_t             not_found;

    now = time(SS_NULL);
    not_found = SS_TRUE;
    list = ss_dns_get_entries(service, dn);
    ss_linked_node_t *node, *next;
    for (
        node = list->head->next;
        node != list->tail;
        node = next
    ) {
        next = node->next;
        temp = (ss_dns_record_t *)node->data;
        if (not_found && ss_addr_equal(temp->addr, addr)) {
            not_found = SS_FALSE;
            temp->expire = now + ttl;
        } else if (temp->expire < now) {
            ss_dns_entry_release(temp);
            ss_linked_list_remove(node);
        }
    }
    if (not_found) {
        entry = ss_dns_entry_create();
        entry->expire = now + ttl;
        entry->addr = addr;
        ss_linked_list_prepend(list, entry);
    }
}

typedef struct ss_dns_rr_s ss_dns_rr_t;

struct ss_dns_header_s {
    ss_uint16_t   tid;
    ss_uint16_t   flags;
    ss_uint16_t   nqueries;
    ss_uint16_t   nanswers;
    ss_uint16_t   nauthorities;
    ss_uint16_t   nadditionals;
};

static const ss_uint16_t SS_DNS_FLAG_EMPTY       = 0x0000;
static const ss_uint16_t SS_DNS_FLAG_QR          = 0x8000; // QR query (0) | response (1)
static const ss_uint16_t SS_DNS_FLAG_OPCODE      = 0x7800; // OPCODE mask (kind of query)
static const ss_uint16_t SS_DNS_FLAG_OP_QUERY    = 0x0000; // OPCODE: query
static const ss_uint16_t SS_DNS_FLAG_OP_IQUERY   = 0x0800; // OPCODE: inverse query
static const ss_uint16_t SS_DNS_FLAG_OP_STATUS   = 0x1000; // OPCODE: server status
static const ss_uint16_t SS_DNS_FLAG_AA          = 0x0400; // authorative answer flag (response only)
static const ss_uint16_t SS_DNS_FLAG_TC          = 0x0200; // truncated flag
static const ss_uint16_t SS_DNS_FLAG_RD          = 0x0100; // recursion desired flag (query only)
static const ss_uint16_t SS_DNS_FLAG_RA          = 0x0080; // recursion available flag (response only)
static const ss_uint16_t SS_DNS_FLAG_Z           = 0x0070; // reserved mask
static const ss_uint16_t SS_DNS_FLAG_RCODE       = 0x000F; // RCODE mask (response code)
static const ss_uint16_t SS_DNS_FLAG_RC_OK       = 0x0000; // RCODE: no error
static const ss_uint16_t SS_DNS_FLAG_RC_EFORMAT  = 0x0001; // RCODE: server failure
static const ss_uint16_t SS_DNS_FLAG_RC_ESRVFLR  = 0x0002; // RCODE: format error
static const ss_uint16_t SS_DNS_FLAG_RC_ENAME    = 0x0003; // RCODE: name error
static const ss_uint16_t SS_DNS_FLAG_RC_ENOTIMP  = 0x0004; // RCODE: not implemented
static const ss_uint16_t SS_DNS_FLAG_RC_EREFUSE  = 0x0005; // RCODE: refused

static void ss_dns_header_query(ss_dns_header_t *header, ss_uint16_t tid, ss_uint16_t nqueries)
{
    memset(header, 0, sizeof(ss_dns_header_t));
    header->tid = tid;
    header->flags = SS_DNS_FLAG_EMPTY
        | (SS_DNS_FLAG_OPCODE & SS_DNS_FLAG_OP_QUERY)
        | SS_DNS_FLAG_RD;
    header->nqueries = nqueries;
}

static ss_io_err_t recv_query(int fd, ss_dns_service_wrapper_t *service, size_t *offset);
static ss_io_err_t recv_answer(int fd, ss_dns_service_wrapper_t *service, size_t *offset);
static ss_io_err_t recv_authority(int fd, ss_dns_service_wrapper_t *service, size_t *offset);
static ss_io_err_t recv_additional(int fd, ss_dns_service_wrapper_t *service, size_t *offset);

static void ss_dns_recv_handler(ss_connection_t *connection)
{
    ss_dns_header_t     header;
    ss_dns_service_wrapper_t   *service;
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

static ss_io_err_t recv_domain_name(int fd, ss_dns_service_wrapper_t *service, size_t *offset, char *domain)
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

static ss_io_err_t recv_query(int fd, ss_dns_service_wrapper_t *service, size_t *offset)
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

static ss_inline
const char *translate_rr_class(ss_uint16_t cls)
{
    switch (cls) {
    case SS_CLASS_IN:
        return "IN";
    case SS_CLASS_CS:
        return "CS";
    case SS_CLASS_CH:
        return "CH";
    case SS_CLASS_HS:
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
    ss_int32_t   ttl;
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

static ss_io_err_t recv_rdata_a(int fd, ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr);
static ss_io_err_t recv_rdata_aaaa(int fd, ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr);
static ss_io_err_t recv_rdata_any(int fd, ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr);

static ss_io_err_t recv_rr(int fd, ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
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
    if (rr->meta.class != SS_CLASS_IN) {
        printf("ERROR: Unsupported RR class %s\n", translate_rr_class(rr->meta.class));
    }
    // receive RDATA
    switch (rr->meta.type) {
    case SS_TYPE_A:
        rc = recv_rdata_a(fd, service, offset, rr);
        break;
    case SS_TYPE_AAAA:
        rc = recv_rdata_aaaa(fd, service, offset, rr);
        break;
    default:
        rc = recv_rdata_any(fd, service, offset, rr);
        break;
    }
    return rc;
}

static ss_io_err_t recv_rdata_a(int fd, ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    ss_addr_t             addr;
    ss_io_err_t           rc;
    ss_dns_record_t       *entry;
    ss_dns_record_list_t  *entry_list;
    time_t                now;

    if (rr->meta.rdlength != sizeof(addr.ipv4.sin_addr))
        return SS_IO_ERROR;
    if ((rc = ss_recv_via_buffer_auto_inc(&addr.ipv4.sin_addr, fd, &service->recv_buffer, offset, sizeof(addr.ipv4.sin_addr))) != SS_IO_OK)
        return rc;
    if (rr->meta.ttl > 0) {
        addr.ipv4.sin_family = AF_INET;
        addr.ipv4.sin_port = 0;
        ss_dns_update(service, rr->name, addr, rr->meta.ttl);
    }
    return SS_IO_OK;
}

static ss_io_err_t recv_rdata_aaaa(int fd, ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    ss_addr_t             addr;
    ss_io_err_t           rc;
    ss_dns_record_t       *entry;
    ss_dns_record_list_t  *entry_list;
    time_t                now;

    if (rr->meta.rdlength != sizeof(addr.ipv6.sin6_addr))
        return SS_IO_ERROR;
    if ((rc = ss_recv_via_buffer_auto_inc(&addr.ipv6.sin6_addr, fd, &service->recv_buffer, offset, sizeof(addr.ipv6.sin6_addr))) != SS_IO_OK)
        return rc;
    if (rr->meta.ttl > 0) {
        addr.ipv6.sin6_family = AF_INET6;
        addr.ipv6.sin6_port = 0;
        ss_dns_update(service, rr->name, addr, rr->meta.ttl);
    }
    return SS_IO_OK;
}

static ss_io_err_t recv_rdata_aaaa(int fd, ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr);

static ss_io_err_t recv_rdata_any(int fd, ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    return ss_recv_via_buffer_auto_inc(SS_NULL, fd, &service->recv_buffer, offset, rr->meta.rdlength);
}

static ss_io_err_t recv_answer(int fd, ss_dns_service_wrapper_t *service, size_t *offset)
{
    int          rc;
    // receive NAME
    if ((rc = recv_domain_name(fd, service, offset, SS_NULL)) != 0) {
        return rc;
    }
}

static ss_io_err_t recv_authority(int fd, ss_dns_service_wrapper_t *service, size_t *offset)
{
}

static ss_int8_t recv_additional(int fd, ss_dns_service_wrapper_t *service, size_t *offset)
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
    ss_dns_service_wrapper_t   *service;
    service = GET_CONTEXT(connection);
    return !service->dead;
}

// NOTES about TID (transaction-ID):
// We use service.tid_flags to mark the occupations of TIDs. The flag has 65536
// bits, representing TID 0 to 65535. The flag is seperated into 4096 segments,
// with each segment representing 16 TIDs. For example, segment 0 represents TID
// 0 to 15, segment 1 represents TID 16 to 31, etc. For each segment, if stored
// in big-endian format, the lowest bit represents the minimum TID, while the
// highest bit represents the maximum TID. For example, for segment 0, the value
// 0x4005 represents that TID 16, 18 and 30 are occupied, while the others are
// available.

/**
 * @brief Idempotently get an available TID, without marking it as occupied.
 * 
 * @param service the DNS service instance
 * @param result the result TID
 * @return ss_bool_t True if successfully. False on failure.
 */
static
ss_bool_t next_tid(ss_dns_service_wrapper_t *service, ss_uint16_t *result)
{
    ss_uint16_t  i;
    ss_uint16_t  tid, seg, flag, temp;
    ss_uint16_t *flags;
    flags = service->tid_flags;
    seg = service->next_tid >> 4;
    for (i = 0; i < 0x1000; i++) {
        flag = flags[seg];
        if (flag == 0xFFFF) {
            seg = (seg + 1) & 0x0FFF;
            continue;
        }
        for (tid = 0; tid < 0x0010; tid++) {
            if ((temp = flag | (1 << tid)) == flag) continue;
            *result = seg | tid;
            return SS_TRUE;
        }
        return SS_FALSE;
    }
    return SS_FALSE;
}

/**
 * @brief Get an available TID, and mark it as occupied.
 * 
 * @param service the DNS service instance
 * @param result the result TID
 * @return ss_bool_t True if successfully. False on failure.
 */
static
ss_bool_t allocate_tid(ss_dns_service_wrapper_t *service, ss_uint16_t *result)
{
    ss_uint16_t  i;
    ss_uint16_t  tid, seg, flag, temp;
    ss_uint16_t *flags;
    flags = service->tid_flags;
    seg = service->next_tid >> 4;
    for (i = 0; i < 0x1000; i++) {
        flag = flags[seg];
        if (flag == 0xFFFF) {
            seg = (seg + 1) & 0x0FFF;
            continue;
        }
        for (tid = 0; tid < 0x0010; tid++) {
            if ((temp = flag | (1 << tid)) == flag) continue;
            flags[seg] = temp;
            tid = seg | tid;
            service->next_tid = tid + 1;
            *result = tid;
            return SS_TRUE;
        }
        return SS_FALSE;
    }
    return SS_FALSE;
}

/**
 * @brief Mark a TID as occupied.
 * 
 * @param service 
 * @param tid 
 */
static
void occupy_tid(ss_dns_service_wrapper_t *service, ss_uint16_t tid)
{
    service->tid_flags[tid >> 4] |= (1 << (tid & 0xF));
    service->next_tid = tid + 1;
}

/**
 * @brief Mark a TID as available.
 * 
 * @param service 
 * @param tid 
 */
static
void release_tid(ss_dns_service_wrapper_t *service, ss_uint16_t tid)
{
    service->tid_flags[tid >> 4] &= (0xFFFF ^ (1 << (tid & 0xF)));
}

static ss_inline
ss_uint16_t is_tid_free(ss_uint16_t *flags, ss_uint16_t tid)
{
    return flags[tid >> 4] & (1 << (tid & 0xF));
}

static
void encode_domain_name(void *dest, const char *dn, size_t dnlen)
{
    size_t       i;
    size_t       labelsz;
    ss_uint8_t  *data;
    if (dnlen == 0) dnlen = strlen(dn);
    data = dest;
    memcpy(dest + 1, (void *)dn, dnlen);
    data[dnlen + 1] = 0;
    labelsz = 0;
    for (i = dnlen; i > 0; i--) {
        if (data[i] == (ss_uint8_t)'.') {
            data[i] = labelsz;
            labelsz = 0;
        } else {
            labelsz++;
        }
    }
    data[0] = labelsz;
}

