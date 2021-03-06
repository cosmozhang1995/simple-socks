#ifndef _SS_NETWORK_SS_CONNECTION_DEF_H_
#define _SS_NETWORK_SS_CONNECTION_DEF_H_

#include <netinet/in.h>

#include "common/ss_types.h"
#include "network/ss_inet.h"

typedef struct ss_connection_s ss_connection_t;

typedef void      (*ss_connection_recv_handler_t)          (ss_connection_t *connection);
typedef void      (*ss_connection_send_handler_t)          (ss_connection_t *connection);
typedef void      (*ss_connection_destroy_handler_t)       (ss_connection_t *connection);
typedef ss_bool_t (*ss_connection_status_function_t)       (ss_connection_t *connection);

struct ss_connection_s {
    int                                   fd;
    int                                   type;
    ss_addr_t                             address;
    ss_connection_recv_handler_t          recv_handler;
    ss_connection_send_handler_t          send_handler;
    ss_connection_destroy_handler_t       destroy_handler;
    ss_connection_status_function_t       check_status;
    void                                 *context;
};

#endif // _SS_NETWORK_SS_CONNECTION_DEF_H_
