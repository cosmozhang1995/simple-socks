#ifndef _SS_NETWORK_SS_LISTENING_DEF_H_
#define _SS_NETWORK_SS_LISTENING_DEF_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "common/ss_types.h"
#include "network/ss_connection_def.h"
#include "network/ss_inet.h"

typedef struct ss_listening_s ss_listening_t;

typedef ss_bool_t (*ss_listening_accept_handler_t)   (ss_connection_t *connection);
typedef void      (*ss_listening_destroy_handler_t)  (ss_listening_t *listening);
typedef ss_bool_t (*ss_listening_status_function_t)  (ss_listening_t *listening);


struct ss_listening_s {
    int                                fd;
    int                                type;
    ss_addr_t                          address;

    /**
     * @brief Handler after a client connection is accepted.
     * @param connection the established connection.
     * @returns False to deny the establish connection. Otherwise, connection must be properly initialized.
     */
    ss_listening_accept_handler_t      accpet_handler;   
    
    /**
     * @brief Handler before after the listening is closed and before it is destroyed.
     * @param listening this listening structure
     */
    ss_listening_destroy_handler_t     destroy_handler;

    void                              *context;
};

#endif // _SS_NETWORK_SS_LISTENING_DEF_H_
