#ifndef _SS_NETWORK_SS_LISTENING_DEF_H_
#define _SS_NETWORK_SS_LISTENING_DEF_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "common/ss_types.h"
#include "network/ss_connection_def.h"
#include "network/ss_inet.h"

typedef struct ss_listening_s ss_listening_t;

typedef ss_bool_t (*ss_listening_accept_handler_t)   (ss_listening_t *listening, ss_connection_t *connection);
typedef void      (*ss_listening_destroy_handler_t)  (ss_listening_t *listening);
typedef ss_bool_t (*ss_listening_status_function_t)  (ss_listening_t *listening);


struct ss_listening_s {
    int                                fd;
    int                                type;
    ss_addr_t                          address;
    ss_listening_accept_handler_t      accpet_handler;   
    ss_listening_destroy_handler_t     destroy_handler;

    void                              *context;
};

#endif // _SS_NETWORK_SS_LISTENING_DEF_H_
