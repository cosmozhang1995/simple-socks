#ifndef __SS_SERVER_H__
#define __SS_SERVER_H__

#include "common/ss_types.h"

typedef void*(*ssbs_create_client_context_function_t)(void *server_config);
typedef void*(*ssbs_release_client_context_function_t)();
typedef void(*ssbs_client_recv_function_t)(int fd, void *context);
typedef void(*ssbs_client_send_function_t)(int fd, void *context);
typedef ss_bool_t(*ssbs_check_client_status_function_t)(void *context);

typedef struct {
    void *server_config;
    ssbs_create_client_context_function_t create_client_context_function;
    ssbs_release_client_context_function_t release_client_context_function;
    ssbs_client_recv_function_t client_recv_handler;
    ssbs_client_send_function_t client_send_handler;
    ssbs_check_client_status_function_t check_client_status_function;
} server_config_t;

int ss_basic_server(server_config_t config);

#endif
