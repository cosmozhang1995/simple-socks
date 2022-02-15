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

#include "common/ss_defs.h"
#include "util/ss_ring_buffer.h"
#include "network/ss_listening.h"
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

static void server_destroy_handler(ss_listening_t *listening);

static ss_bool_t accept_handler(ss_listening_t *listening, ss_connection_t *connection);
static void recv_handler(ss_connection_t *connection);
static void send_handler(ss_connection_t *connection);
static void destroy_handler(ss_connection_t *connection);
static ss_bool_t check_client_status(ss_connection_t *connection);

static ss_context_t *create_client_context(ss_config_t *server_config);
static void release_client_context(ss_context_t *context);

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

    return result;
}

int simple_socks5_start(int argc, char *argv[])
{
    ss_config_t             *ss_config = 0;
    ss_cli_arguments_t       args;
    ss_listening_t          *listening;
    ss_addr_t                addr;

    args = get_argumetns(argc, argv);
    ss_config = ss_load_config(args.config_path);
    if (!ss_config) {
        printf("failed to load config from %s\n", args.config_path);
        return -1;
    }

    addr = ss_make_ipv4_addr("0.0.0.0", 1080);
    listening = ss_network_listen(SOCK_STREAM, addr);
    if (!listening) return -1;
    listening->context = ss_config;
    listening->accpet_handler = accept_handler;
    listening->destroy_handler = server_destroy_handler;

    return 0;
}

static ss_inline
ss_config_t * get_server_config(const ss_listening_t *listening)
{
    return (ss_config_t *)listening->context;
}

static ss_inline
ss_context_t * get_client_context(const ss_connection_t *connection)
{
    return (ss_context_t *)connection->context;
}

static void server_destroy_handler(ss_listening_t *listening)
{
    ss_release_config(get_server_config(listening->context));
}

static ss_bool_t accept_handler(ss_listening_t *listening, ss_connection_t *connection)
{
    ss_context_t          *context;

    context = create_client_context(get_server_config(listening));
    connection->context = context;
    connection->recv_handler = recv_handler;
    connection->send_handler = send_handler;
    connection->destroy_handler = destroy_handler;
    connection->check_status = check_client_status;

    return SS_TRUE;
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



static void recv_handler(ss_connection_t *connection)
{
    int           fd;
    ss_context_t *context;
    ss_int8_t     retcode;

    fd = connection->fd;
    context = get_client_context(connection);
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

static void send_handler(ss_connection_t *connection)
{
    int           fd;
    ss_context_t *context;

    fd = connection->fd;
    context = get_client_context(connection);
    ss_ring_buffer_send(fd, &context->write_buffer, context->write_buffer.length);
}

static void destroy_handler(ss_connection_t *connection)
{
    release_client_context(get_client_context(connection));
}

static ss_bool_t check_client_status(ss_connection_t *connection) {
    return get_client_context(connection)->status != SS_STATUS_DEAD
        || get_client_context(connection)->write_buffer.length > 0;
}
