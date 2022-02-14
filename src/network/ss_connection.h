#ifndef _SS_NETWORK_SS_CONNECTION_H_
#define _SS_NETWORK_SS_CONNECTION_H_

#include "common/ss_types.h"

typedef struct ss_connection_s ss_connection_t;

typedef ss_int8_t (*ss_connection_recv_handler_t)          (ss_connection_t *connection);
typedef ss_int8_t (*ss_connection_send_handler_t)          (ss_connection_t *connection);
typedef void      (*ss_connection_release_function_t)      (ss_connection_t *connection);
typedef ss_bool_t (*ss_connection_check_status_function_t) (ss_connection_t *connection);

struct ss_connection_s {
    int                                         fd;
    ss_connection_recv_handler_t                recv_handler;
    ss_connection_send_handler_t                send_handler;
    ss_connection_release_function_t            release;
    ss_connection_check_status_function_t       check_status;
};

ss_bool_t ss_network_add_conenction(ss_connection_t *);

#endif // _SS_NETWORK_SS_CONNECTION_H_
