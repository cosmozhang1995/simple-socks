#include "server/simple_socks5_server.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "server/server.h"
#include "util/ss_ring_buffer.h"
#include "socks5/ss_context.h"
#include "socks5/ss_version.h"
#include "socks5/ss_auth_method.h"
#include "socks5/ss_auth.h"
#include "socks5/ss_server_helper.h"
#include "socks5/ss_negotiation.h"
#include "socks5/ss_process_def.h"

typedef struct {
    const char *config_path;
} ss_cli_arguments_t;

static ss_context_t *create_client_context(ss_config_t *server_config);
static void release_client_context(ss_context_t *context);
static void client_receive_function(int fd, ss_context_t *context);
static void client_send_function(int fd, ss_context_t *context);
static ss_bool_t check_client_status(ss_context_t *context);

static ss_cli_arguments_t get_argumetns(int argc, char *argv[])
{
    int i;
    ss_cli_arguments_t result;
    result.config_path = "/etc/socks/socks.conf";

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            result.config_path = argv[++i];
        }
    }
}

int simple_socks5_server(int argc, char *argv[])
{
    int exit_code;
    ss_config_t *ss_config = 0;
    server_config_t config;
    ss_cli_arguments_t args;

    args = get_argumetns(argc, argv);
    ss_config = ss_load_config(args.config_path);
    if (!ss_config) return -1;

    config.server_config = ss_config;
    config.create_client_context_function = (ssbs_create_client_context_function_t)create_client_context;
    config.release_client_context_function = (ssbs_release_client_context_function_t)release_client_context;
    config.client_recv_handler = (ssbs_client_recv_function_t)client_receive_function;
    config.client_send_handler = (ssbs_client_send_function_t)client_send_function;
    config.check_client_status_function = (ssbs_check_client_status_function_t)check_client_status;

    exit_code = ss_basic_server(config);

    if (ss_config) ss_release_config(ss_config);
    return exit_code;
}

static ss_context_t *create_client_context(ss_config_t *server_config)
{
    ss_context_t *context = malloc(sizeof(ss_context_t));
    ss_context_initialize(context, server_config);
    return context;
}

static void release_client_context(ss_context_t *context)
{
    ss_context_uninitialize(context);
    free(context);
}

static void client_receive_function(int fd, ss_context_t *context)
{
    ss_int8_t retcode;
    while (1) {
        switch (context->status) {
        case SS_STATUS_NEGOTIATING:
            retcode = ss_negotiate_process(fd, context);
            break;
        case SS_STATUS_NEGOTIATED:
            retcode = ss_auth_begin(context);
            break;
        case SS_STATUS_AUTHENTICATING:
            retcode = ss_auth_process(fd, context);
            break;
        case SS_STATUS_AUTHENTICATED:
            retcode = ss_auth_finish(context);
            break;
        default:
            break;
        }
        // this process is done. continue to try a next process
        if (retcode == SS_PROCESS_DONE)
            continue;
        // this process need more data. break to return to main loop and wait for more data to come.
        if (retcode == SS_PROCESS_AGAIN)
            break;
        // got error. mark as dead and return to main loop.
        ss_client_kill(context);
        break;
    }
}

static void client_send_function(int fd, ss_context_t *context)
{
    ss_ring_buffer_send(fd, &context->write_buffer, context->write_buffer.length);
}

static ss_bool_t check_client_status(ss_context_t *context) {
    return context->status != SS_STATUS_DEAD || context->write_buffer.length > 0;
}
