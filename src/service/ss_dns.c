#include "service/ss_dns.h"

#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <stdio.h>

#include "common/ss_defs.h"
#include "common/ss_time.h"
#include "util/ss_strmap.h"
#include "util/ss_linked_list.h"
#include "util/ss_packet_buffer.h"
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

#define DEFAULT_MIN_REQUEST_INTERVAL 600

#define RECV_BUFFER_PACKSIZE 0x800
#define RECV_BUFFER_PACKCAP  4
#define SEND_BUFFER_PACKSIZE 0x800
#define SEND_BUFFER_PACKCAP  16

typedef struct ss_dns_service_wrapper_s ss_dns_service_wrapper_t;

struct ss_dns_service_wrapper_s {
    ss_dns_service_t    export;
    ss_strmap_t         map;
    ss_uint16_t         tid_flags[SS_UINT16_MAX >> 4];
    ss_uint16_t         next_tid;
    ss_bool_t           dead;
    ss_packet_buffer_t  recv_buffer;
    ss_packet_buffer_t  send_buffer;
};

static void ss_dns_recv_handler(ss_connection_t *connection);
static void ss_dns_send_handler(ss_connection_t *connection);
static void ss_dns_destroy_handler(ss_connection_t *connection);
static ss_bool_t ss_dns_check_status(ss_connection_t *connection);

#define GET_CONTEXT(connection) ((ss_dns_service_wrapper_t *)connection->context)

typedef struct ss_dns_entry_s ss_dns_entry_t;

typedef ss_linked_list_t ss_dns_record_list_t;

typedef struct ss_dns_record_s ss_dns_record_t;

#define SS_DNS_HEADER_SIZE 12

typedef struct ss_dns_header_s ss_dns_header_t;

typedef struct ss_dns_rr_s ss_dns_rr_t;

#define SS_DNS_RR_META_SIZE 10

typedef struct ss_dns_rr_meta_s ss_dns_rr_meta_t;

typedef union ss_dns_rdata_u ss_dns_rdata_t;

struct ss_dns_header_s {
    ss_uint16_t           tid;
    ss_uint16_t           flags;
    ss_uint16_t           nqueries;
    ss_uint16_t           nanswers;
    ss_uint16_t           nauthorities;
    ss_uint16_t           nadditionals;
};

struct ss_dns_entry_s {
    ss_time_t             last_request;
    ss_dns_record_list_t  list;
};

union ss_dns_rdata_u {
    ss_addr_t            addr;
    char                 cname[SS_DNML + 2];
};

struct ss_dns_rr_meta_s {
    ss_uint16_t  type;
    ss_uint16_t  class;
    ss_int32_t   ttl;
    ss_uint16_t  rdlength;
};

struct ss_dns_rr_s {
    char                 name[SS_DNML + 2];
    ss_dns_rr_meta_t     meta;
    ss_dns_rdata_t       rdata;
};

struct ss_dns_record_s {
    ss_dns_rdata_t        data;
    ss_uint16_t           type;
    time_t                expire;
};

static ss_bool_t next_tid(ss_dns_service_wrapper_t *service, ss_uint16_t *tid);
static ss_bool_t allocate_tid(ss_dns_service_wrapper_t *service, ss_uint16_t *tid);
static void occupy_tid(ss_dns_service_wrapper_t *service, ss_uint16_t tid);
static void release_tid(ss_dns_service_wrapper_t *service, ss_uint16_t tid);
static ss_inline ss_uint16_t is_tid_free(ss_uint16_t *flags, ss_uint16_t tid);

static void encode_domain_name(void *dest, const char *dn, size_t dnlen);

static void ss_dns_header_query(ss_dns_header_t *header, ss_uint16_t tid, ss_uint16_t nqueries);

static ss_dns_entry_t *ss_dns_entry_create();
static void ss_dns_entry_release(ss_dns_entry_t *entry);
static ss_bool_t ss_dns_entry_map_release_visitor(const char *, ss_variable_t *, void *);

static ss_inline ss_dns_entry_t *ss_dns_get_entry(ss_dns_service_wrapper_t *service, const char *dn);
static ss_inline ss_dns_entry_t *ss_dns_entry(ss_dns_service_wrapper_t *service, const char *dn);


static void ss_dns_header_hton(ss_dns_header_t *header);
static void ss_dns_header_ntoh(ss_dns_header_t *header);


static ss_dns_service_wrapper_t *ss_dns_service_start_inner(ss_addr_t addr);
ss_dns_service_t *ss_dns_service_start(ss_addr_t addr)
{
    return (ss_dns_service_t *)ss_dns_service_start_inner(addr);
}

static void ss_dns_service_stop_inner(ss_dns_service_wrapper_t *);
void ss_dns_service_stop(ss_dns_service_t *service)
{
    ss_dns_service_stop_inner((ss_dns_service_wrapper_t *)service);
}

static ss_io_err_t ss_dns_fetch_inner(ss_dns_service_wrapper_t *service, const char *domain_name, ss_callback_context_t *callback);
ss_io_err_t ss_dns_fetch(ss_dns_service_t *service, const char *domain_name, ss_callback_context_t *callback)
{
    return ss_dns_fetch_inner((ss_dns_service_wrapper_t *)service, domain_name, callback);
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


ss_dns_service_wrapper_t *ss_dns_service_create(ss_addr_t addr);
void ss_dns_service_release(ss_dns_service_wrapper_t *service);

void ss_dns_service_initialize(ss_dns_service_wrapper_t *service)
{
    memset(service, 0, sizeof(ss_dns_service_wrapper_t));
    ss_strmap_initialize(&service->map);
    service->export.min_request_interval = DEFAULT_MIN_REQUEST_INTERVAL;
    ss_packet_buffer_initialize(&service->recv_buffer, RECV_BUFFER_PACKSIZE, RECV_BUFFER_PACKCAP);
    ss_packet_buffer_initialize(&service->send_buffer, SEND_BUFFER_PACKSIZE, SEND_BUFFER_PACKCAP);
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
    ss_strmap_foreach(&service->map, ss_dns_entry_map_release_visitor, SS_NULL);
    ss_strmap_uninitialize(&service->map);
    ss_packet_buffer_uninitialize(&service->recv_buffer);
    ss_packet_buffer_uninitialize(&service->send_buffer);
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
    return ss_dns_service_create(addr);
}

static
void ss_dns_service_stop_inner(ss_dns_service_wrapper_t *service)
{
    ss_dns_service_disconnect(service);
}

#define SS_DNS_QUESTION_TAIL_SIZE 4

typedef struct ss_dns_question_tail_s ss_dns_question_tail_t;

struct ss_dns_question_tail_s {
    ss_uint16_t qtype;
    ss_uint16_t qclass;
};

static void ss_dns_question_tail_hton(ss_dns_question_tail_t *tail);
static void ss_dns_question_tail_ntoh(ss_dns_question_tail_t *tail);

/**
 * @brief Fetch a DNS resolved address for the specified domain name. If no local storage, start a new request.
 * 
 * @param service service instance
 * @param dn domain name
 * @param callback callback
 * @return ss_io_err_t OK for success. EAGIAN if requesting. Otherwise for failure.
 */
static
ss_io_err_t ss_dns_fetch_inner(ss_dns_service_wrapper_t *service, const char *dn, ss_callback_context_t *callback)
{
    ss_io_err_t      rc;
    ss_addr_t        addr;
    if (ss_dns_get_inner(service, dn, &addr)) {
        return SS_IO_OK;
    }
    if ((rc = ss_dns_resolve_start_inner(service, dn)) != SS_IO_OK) {
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
ss_io_err_t ss_dns_resolve_start_inner(ss_dns_service_wrapper_t *service, const char *dn, ss_callback_context_t *callback)
{
    ss_variable_t               var;
    ss_dns_entry_t             *entry;
    ss_uint16_t                 tid;
    ss_uint8_t                  data[
                                    SS_DNS_HEADER_SIZE
                                    + SS_DNML + 2
                                    + SS_DNS_QUESTION_TAIL_SIZE
                                    + SS_DNML + 2
                                    + SS_DNS_QUESTION_TAIL_SIZE];
    ss_dns_header_t            *header;
    ss_dns_question_tail_t     *ques_a_tail;
    ss_dns_question_tail_t     *ques_aaaa_tail;
    size_t                      dnlen;
    void                       *ques_a_dn;
    void                       *ques_aaaa_dn;
    size_t                      i;
    ss_uint8_t                  labelsz;
    size_t                      reqlen;
    ss_io_err_t                 rc;
    ss_time_t                   now;
    ss_packet_block_t           reqpack;

    if (service->dead) return SS_IO_ERROR;
    time(&now);
    entry == ss_dns_get_entry(service, dn);
    if (entry && entry->last_request + service->export.min_request_interval > now) {
        return SS_IO_EAGAIN;
    }
    dnlen = strlen(dn);
    if (dnlen > SS_DNML) {
        return SS_IO_ERROR;
    }
    reqlen = 0;
    header = (ss_dns_header_t *)data;
    reqlen += SS_DNS_HEADER_SIZE;
    ques_a_dn = (void *)(data + reqlen);
    reqlen += dnlen + 2;
    ques_a_tail = (ss_dns_question_tail_t *)(data + reqlen);
    reqlen += SS_DNS_QUESTION_TAIL_SIZE;
    ques_aaaa_dn = (void *)(data + reqlen);
    reqlen += dnlen + 2;
    ques_aaaa_tail = (ss_dns_question_tail_t *)(data + reqlen);
    reqlen += SS_DNS_QUESTION_TAIL_SIZE;
    // initialize header data
    if (!next_tid(service, &tid)) {
        return SS_IO_EAGAIN;
    }
    ss_dns_header_query(header, tid, 1);
    ss_dns_header_hton(header);
    // initialize A question QNAME
    encode_domain_name(ques_a_dn, dn, dnlen);
    // initialize A question tail
    ques_a_tail->qtype = SS_TYPE_AAAA;
    ques_a_tail->qclass = SS_CLASS_IN;
    ss_dns_question_tail_hton(ques_a_tail);
    // // initialize AAAA question QNAME, which is same to A question QNAME
    // memcpy((void *)ques_aaaa_dn, (void *)ques_a_dn, dnlen + 2);
    // // initialize AAAA question tail
    // ques_aaaa_tail->qtype = SS_TYPE_AAAA;
    // ques_aaaa_tail->qclass = SS_CLASS_IN;
    // ss_dns_question_tail_hton(ques_aaaa_tail);
    // write question
    if (!ss_packet_buffer_write_back(&service->send_buffer, data, SS_NULL, reqlen)) {
        return SS_IO_EAGAIN;
    }
    reqpack.size = reqlen;
    if (!ss_packet_buffer_push_back(&service->send_buffer, reqpack)) {
        return SS_IO_EAGAIN;
    }
    occupy_tid(service, tid);
    if (entry == SS_NULL) entry = ss_dns_entry(service, dn);
    entry->last_request = now;
    return SS_IO_OK;
}

static
ss_bool_t ss_dns_get_inner(ss_dns_service_wrapper_t *service, const char *dn, ss_addr_t *addr)
{
    ss_dns_entry_t   *entry;
    ss_linked_node_t *node;
    ss_dns_record_t  *record;
    ss_time_t         now;

    time(&now);
    if ((entry = ss_dns_get_entry(service, dn)) == SS_NULL) {
        return SS_FALSE;
    }
    while (SS_TRUE) {
        node = ss_linked_list_pop_node_front(&entry->list);
        if (node == SS_NULL) {
            return SS_FALSE;
        }
        record = (ss_dns_record_t *)node->data;
        if (record->expire < now) {
            ss_linked_list_remove(node);
            continue;
        }
        ss_linked_list_append_node(&entry->list, node);
        if (record->type == SS_TYPE_CNAME) {
            return ss_dns_get_inner(service, record->data.cname, addr);
        } else {
            *addr = record->data.addr;
        }
        return SS_TRUE;
    }
    return SS_FALSE;
}

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
    entry->last_request = 0;
    ss_dns_list_initialize(&entry->list);
    return entry;
}

static void ss_dns_entry_release(ss_dns_entry_t *entry)
{
    ss_dns_list_uninitialize(&entry->list);
    free(entry);
}

static ss_bool_t ss_dns_entry_map_release_visitor(const char *dn, ss_variable_t *var, void *arg)
{
    ss_dns_entry_release((ss_dns_entry_t *)var->ptr);
    return SS_TRUE;
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

static ss_bool_t ss_dns_record_compare(ss_dns_record_t *r1, ss_dns_record_t *r2)
{
    if (r1->type != r2->type) return SS_FALSE;
    switch (r1->type) {
        case SS_TYPE_A:
        case SS_TYPE_AAAA:
            return ss_addr_equal(r1->data.addr, r2->data.addr);
        case SS_TYPE_CNAME:
            return strcmp(r1->data.cname, r2->data.cname) == 0;
    }
    return SS_TRUE;
}

static ss_bool_t ss_dns_record_compare_rr(ss_dns_record_t *r, ss_dns_rr_t *rr)
{
    if (r->type != rr->meta.type) return SS_FALSE;
    switch (r->type) {
        case SS_TYPE_A:
        case SS_TYPE_AAAA:
            return ss_addr_equal(r->data.addr, rr->rdata.addr);
        case SS_TYPE_CNAME:
            return strcmp(r->data.cname, rr->rdata.cname) == 0;
    }
    return SS_TRUE;
}

static void ss_dns_update(ss_dns_service_wrapper_t *service, ss_dns_rr_t *rr)
{
    ss_dns_entry_t        *entry;
    ss_dns_record_list_t  *list;
    ss_dns_record_t       *record, *temp;
    time_t                now;
    ss_bool_t             not_found;

    time(&now);
    not_found = SS_TRUE;
    entry = ss_dns_entry(service, rr->name);
    list = &entry->list;
    ss_linked_node_t *node, *next;
    for (
        node = list->head->next;
        node != list->tail;
        node = next
    ) {
        next = node->next;
        temp = (ss_dns_record_t *)node->data;
        if (not_found && ss_dns_record_compare_rr(temp, rr)) {
            not_found = SS_FALSE;
            temp->expire = now + rr->meta.ttl;
        } else if (temp->expire < now) {
            ss_dns_record_release(temp);
            ss_linked_list_remove(node);
        }
    }
    if (not_found) {
        record = ss_dns_record_create();
        record->expire = now + rr->meta.ttl;
        record->type = rr->meta.type;
        switch (rr->meta.type) {
            case SS_TYPE_A:
            case SS_TYPE_AAAA:
                record->data.addr = rr->rdata.addr;
                break;
            case SS_TYPE_CNAME:
                strcpy(record->data.cname, rr->rdata.cname);
                break;
        }
        ss_linked_list_prepend(list, record);
    }
}

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

static ss_io_err_t read_query(ss_dns_service_wrapper_t *service, size_t *offset);
static ss_io_err_t read_answer(ss_dns_service_wrapper_t *service, size_t *offset);
static ss_io_err_t read_authority(ss_dns_service_wrapper_t *service, size_t *offset);
static ss_io_err_t read_additional(ss_dns_service_wrapper_t *service, size_t *offset);

static void ss_dns_recv_handler(ss_connection_t *connection)
{
    ss_dns_header_t             header;
    ss_dns_service_wrapper_t   *service;
    ss_uint16_t                 i;
    size_t                      offset;
    ss_io_err_t                 rc;
    ss_packet_t                 packet;

    service = GET_CONTEXT(connection);
    if (service->dead) return;
    rc = SS_IO_OK;
    offset = 0;
    if ((rc = ss_packet_buffer_recv_back(connection->fd, &service->recv_buffer, RECV_BUFFER_PACKSIZE)) != SS_IO_OK) {
        if (rc < 0) service->dead = SS_FALSE;
        return;
    }
    if (!ss_packet_buffer_read_front(&header, &service->recv_buffer, &offset, SS_DNS_HEADER_SIZE)) {
        rc = SS_IO_ERROR;
        goto _l_error;
    }
    ss_dns_header_ntoh(&header);
    if (header.flags & SS_DNS_FLAG_RCODE != SS_DNS_FLAG_RC_OK) {
        goto _l_error;
    }
    for (i = 0; i < header.nqueries; i++) {
        if ((rc = read_query(service, &offset)) != SS_IO_OK) goto _l_error;
    }
    for (i = 0; i < header.nanswers; i++) {
        if ((rc = read_answer(service, &offset)) != SS_IO_OK) goto _l_error;
    }
    for (i = 0; i < header.nauthorities; i++) {
        if ((rc = read_authority(service, &offset)) != SS_IO_OK) goto _l_error;
    }
    for (i = 0; i < header.nadditionals; i++) {
        if ((rc = read_additional(service, &offset)) != SS_IO_OK) goto _l_error;
    }
_l_success:
    ss_packet_buffer_pop_front(&service->recv_buffer, SS_NULL);
    release_tid(service, header.tid);
    return;
_l_error:
    ss_packet_buffer_pop_front(&service->recv_buffer, SS_NULL);
    if (rc < 0) service->dead = SS_FALSE;
    return;
}

static ss_io_err_t read_domain_name_with_total_length(ss_dns_service_wrapper_t *service, size_t *offset, char *domain, ss_uint16_t *total_length)
{
    ss_uint8_t   label_length;
    ss_io_err_t  rc;
    ss_size_t    pointer_offset;

    rc = SS_IO_OK;
    while (SS_TRUE) {
        if (!ss_packet_buffer_read_front(&label_length, &service->recv_buffer, offset, sizeof(label_length))) {
            rc = SS_IO_ERROR;
            goto _l_end;
        }
        if (label_length == 0) {
            rc = SS_IO_OK;
            goto _l_end;
        }
        if (label_length & 0xC0) {
            goto _l_compressed;
        }
        if (*total_length + (ss_uint16_t)label_length > SS_DNML) {
            printf("ERROR: domain name exceeds length restriction\n");
            rc = SS_IO_ERROR;
            goto _l_end;
        }
        if (!ss_packet_buffer_read_front(
                (void *)(domain ? domain + *total_length : SS_NULL),
                &service->recv_buffer, offset, (size_t)label_length)
        ) {
            rc = SS_IO_ERROR;
            goto _l_end;
        }
        *total_length += (ss_uint16_t)label_length;
        if (domain) domain[(*total_length)++] = '.';
    }
_l_compressed:
    pointer_offset = ((ss_size_t)(label_length & 0x3F)) << 8;
    if (!ss_packet_buffer_read_front(&label_length, &service->recv_buffer, offset, sizeof(label_length))) {
        rc = SS_IO_ERROR;
        goto _l_end;
    }
    pointer_offset |= label_length;
    rc = read_domain_name_with_total_length(service, &pointer_offset, domain, total_length);
    return rc;
_l_end:
    if (domain) {
        if (*total_length != 0) {
            domain[*total_length - 1] = '\0';
        } else {
            domain[0] = '\0';
        }
    }
    return rc;
}

static ss_io_err_t read_domain_name(ss_dns_service_wrapper_t *service, size_t *offset, char *domain)
{
    ss_uint16_t total_length;
    total_length = 0;
    return read_domain_name_with_total_length(service, offset, domain, &total_length);
}

static ss_io_err_t read_query(ss_dns_service_wrapper_t *service, size_t *offset)
{
    int          rc;
    // receive QNAME
    if ((rc = read_domain_name(service, offset, SS_NULL)) != SS_IO_OK) {
        return rc;
    }
    // receive QTYPE
    if (!ss_packet_buffer_read_front(SS_NULL, &service->recv_buffer, offset, sizeof(ss_uint16_t))) {
        return rc;
    }
    // receive QCLASS
    if (!ss_packet_buffer_read_front(SS_NULL, &service->recv_buffer, offset, sizeof(ss_uint16_t))) {
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

static void ss_dns_rr_meta_hton(ss_dns_rr_meta_t *meta);
static void ss_dns_rr_meta_ntoh(ss_dns_rr_meta_t *meta);

static void ss_dns_rdata_initialize(ss_dns_rdata_t *rdata)
{
    memset(rdata, 0, sizeof(ss_dns_rdata_t));
}

static void ss_dns_rdata_uninitialize(ss_dns_rdata_t *rdata)
{
}

static void ss_dns_rr_initialize(ss_dns_rr_t *rr)
{
    memset((void *)rr, 0, sizeof(rr));
    ss_dns_rdata_initialize(&rr->rdata);
}

static void ss_dns_rr_uninitialize(ss_dns_rr_t *rr)
{
    ss_dns_rdata_uninitialize(&rr->rdata);
}

static ss_io_err_t read_rdata_a(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr);
static ss_io_err_t read_rdata_aaaa(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr);
static ss_io_err_t read_rdata_cname(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr);
static ss_io_err_t read_rdata_any(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr);

static ss_io_err_t read_rr(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    int          rc;
    // receive NAME
    if ((rc = read_domain_name(service, offset, rr->name)) != SS_IO_OK) {
        return rc;
    }
    // receive meta
    if (!ss_packet_buffer_read_front(&rr->meta, &service->recv_buffer, offset, SS_DNS_RR_META_SIZE)) {
        return SS_IO_ERROR;
    }
    ss_dns_rr_meta_ntoh(&rr->meta);
    if (rr->meta.class != SS_CLASS_IN) {
        printf("ERROR: Unsupported RR class %s (%d)\n", translate_rr_class(rr->meta.class), (ss_int32_t)rr->meta.class);
        rc = SS_IO_ERROR;
    }
    // receive RDATA
    switch (rr->meta.type) {
    case SS_TYPE_A:
        rc = read_rdata_a(service, offset, rr);
        break;
    case SS_TYPE_AAAA:
        rc = read_rdata_aaaa(service, offset, rr);
        break;
    case SS_TYPE_CNAME:
        rc = read_rdata_cname(service, offset, rr);
        break;
    default:
        rc = read_rdata_any(service, offset, rr);
        break;
    }
    return rc;
}

static ss_io_err_t read_rdata_a(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    ss_addr_t              addr;
    ss_dns_record_t       *entry;
    ss_dns_record_list_t  *entry_list;

    if (rr->meta.rdlength != SS_IPV4_ADDR_SIZE) {
        printf("RDATA for A record expect size %lu, but got %lu\n", (ss_size_t)SS_IPV4_ADDR_SIZE, (ss_size_t)rr->meta.rdlength);
        return SS_IO_ERROR;
    }
    if (!ss_packet_buffer_read_front(&addr.ipv4.sin_addr, &service->recv_buffer, offset, SS_IPV4_ADDR_SIZE)) {
        return SS_IO_ERROR;
    }
    addr.ipv4.sin_family = AF_INET;
    addr.ipv4.sin_port = 0;
    rr->rdata.addr = addr;
    return SS_IO_OK;
}

static ss_io_err_t read_rdata_aaaa(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    ss_addr_t              addr;
    ss_dns_record_t       *entry;
    ss_dns_record_list_t  *entry_list;

    if (rr->meta.rdlength != SS_IPV6_ADDR_SIZE) {
        printf("RDATA for AAAA record expect size %lu, but got %lu\n", (ss_size_t)SS_IPV6_ADDR_SIZE, (ss_size_t)rr->meta.rdlength);
        return SS_IO_ERROR;
    }
    if (!ss_packet_buffer_read_front(&addr.ipv6.sin6_addr, &service->recv_buffer, offset, SS_IPV6_ADDR_SIZE)) {
        return SS_IO_ERROR;
    }
    addr.ipv6.sin6_family = AF_INET6;
    addr.ipv6.sin6_port = 0;
    rr->rdata.addr = addr;
    return SS_IO_OK;
}

static ss_io_err_t read_rdata_cname(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    ss_io_err_t            rc;
    ss_dns_record_t       *entry;
    ss_dns_record_list_t  *entry_list;
    ss_size_t              init_offset;

    init_offset = *offset;
    if ((rc = read_domain_name(service, offset, rr->rdata.cname)) != SS_IO_OK)
        return rc;
    if (*offset - init_offset != rr->meta.rdlength)
        return SS_IO_ERROR;
    return SS_IO_OK;
}

static ss_io_err_t read_rdata_any(ss_dns_service_wrapper_t *service, size_t *offset, ss_dns_rr_t *rr)
{
    if (!ss_packet_buffer_read_front(SS_NULL, &service->recv_buffer, offset, rr->meta.rdlength))
        return SS_IO_ERROR;
    return SS_IO_OK;
}

static ss_io_err_t read_answer(ss_dns_service_wrapper_t *service, size_t *offset)
{
    ss_dns_rr_t rr;
    ss_io_err_t rc;
    if ((rc = read_rr(service, offset, &rr)) != SS_IO_OK)
        return rc;
    switch (rr.meta.type) {
    case SS_TYPE_A:
    case SS_TYPE_AAAA:
    case SS_TYPE_CNAME:
        if (rr.meta.ttl > 0) {
            ss_dns_update(service, &rr);
        }
        break;
    default:
        break;
    }
    return rc;
}

static ss_io_err_t read_authority(ss_dns_service_wrapper_t *service, size_t *offset)
{
    ss_dns_rr_t rr;
    return read_rr(service, offset, &rr);
}

static ss_int8_t read_additional(ss_dns_service_wrapper_t *service, size_t *offset)
{
    ss_dns_rr_t rr;
    return read_rr(service, offset, &rr);
}

static void ss_dns_send_handler(ss_connection_t *connection)
{
    ss_packet_buffer_t      *buffer;
    buffer = &GET_CONTEXT(connection)->send_buffer;
    while (buffer->packet_count != 0)
        ss_packet_buffer_send_front(connection->fd, buffer);
}

static void ss_dns_destroy_handler(ss_connection_t *connection)
{
    ss_dns_service_release(GET_CONTEXT(connection));
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


static void ss_dns_header_hton(ss_dns_header_t *header)
{
    header->tid = htons(header->tid);
    header->flags = htons(header->flags);
    header->nqueries = htons(header->nqueries);
    header->nanswers = htons(header->nanswers);
    header->nauthorities = htons(header->nauthorities);
    header->nadditionals = htons(header->nadditionals);
}

static void ss_dns_header_ntoh(ss_dns_header_t *header)
{
    header->tid = ntohs(header->tid);
    header->flags = ntohs(header->flags);
    header->nqueries = ntohs(header->nqueries);
    header->nanswers = ntohs(header->nanswers);
    header->nauthorities = ntohs(header->nauthorities);
    header->nadditionals = ntohs(header->nadditionals);
}


static void ss_dns_question_tail_hton(ss_dns_question_tail_t *tail)
{
    tail->qtype = htons(tail->qtype);
    tail->qclass = htons(tail->qclass);
}

static void ss_dns_question_tail_ntoh(ss_dns_question_tail_t *tail)
{
    tail->qtype = ntohs(tail->qtype);
    tail->qclass = ntohs(tail->qclass);
}


static void ss_dns_rr_meta_hton(ss_dns_rr_meta_t *meta)
{
    meta->type = htons(meta->type);
    meta->class = htons(meta->class);
    meta->ttl = htonl(meta->ttl);
    meta->rdlength = htons(meta->rdlength);
}

static void ss_dns_rr_meta_ntoh(ss_dns_rr_meta_t *meta)
{
    meta->type = ntohs(meta->type);
    meta->class = ntohs(meta->class);
    meta->ttl = ntohl(meta->ttl);
    meta->rdlength = ntohs(meta->rdlength);
}

