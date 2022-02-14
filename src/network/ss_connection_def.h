#ifndef _SS_NETWORK_SS_CONNECTION_DEF_H_
#define _SS_NETWORK_SS_CONNECTION_DEF_H_

#include <netinet/in.h>

#include "common/ss_types.h"

typedef struct ss_connection_s ss_connection_t;

typedef ss_int8_t (*ss_connection_recv_handler_t)          (ss_connection_t *connection);
typedef ss_int8_t (*ss_connection_send_handler_t)          (ss_connection_t *connection);
typedef void      (*ss_connection_destroy_handler_t)       (ss_connection_t *connection);
typedef ss_bool_t (*ss_connection_status_function_t)       (ss_connection_t *connection);

struct ss_connection_s {
    int                                   fd;
    int                                   domain;
    int                                   type;
    struct sockaddr                       address;
    ss_connection_recv_handler_t          recv_handler;
    ss_connection_send_handler_t          send_handler;
    ss_connection_destroy_handler_t       destroy_handler;
    ss_connection_status_function_t       check_status;
};

#endif // _SS_NETWORK_SS_CONNECTION_DEF_H_
