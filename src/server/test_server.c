#include "server/test_server.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <memory.h>

#include "util/ss_ring_buffer.h"
#include "network/ss_listening.h"
#include "network/ss_inet.h"
#include "network/ss_connection.h"
#include "testing/ss_testing_interface.h"

typedef struct client_context_s {
    ss_ring_buffer_t        buffer;
    size_t                  nbytes_sent;
    ss_testing_session_t    session;
    ss_bool_t               dead;
} client_context_t;

static client_context_t *create_client_context(void *server);
static void release_client_context(client_context_t *context);
static void client_receive_function(int connfd, client_context_t *context);
static void client_send_function(int connfd, client_context_t *context);
static void client_send_hexcode_function(int connfd, client_context_t *context);

static void server_destroy_handler(ss_listening_t *listening);

static ss_bool_t accept_handler(ss_listening_t *listening, ss_connection_t *connection);
static void recv_handler(ss_connection_t *connection);
static void send_handler(ss_connection_t *connection);
static void destroy_handler(ss_connection_t *connection);
static ss_bool_t check_status(ss_connection_t *connection);

int test_server_start()
{
    ss_addr_t             addr;
    ss_listening_t       *listening;
    ss_testing_server_t  *server;

    server = ss_testing_start();

    addr = ss_make_ipv4_addr("0.0.0.0", 2080);
    listening = ss_network_listen(SOCK_STREAM, addr);
    if (!listening) return -1;
    listening->accpet_handler = accept_handler;
    listening->destroy_handler = server_destroy_handler;
    listening->context = server;

    return 0;
}

static void server_destroy_handler(ss_listening_t *listening)
{
    ss_testing_stop((ss_testing_server_t *)listening->context);
}

static ss_bool_t accept_handler(ss_listening_t *listening, ss_connection_t *connection)
{
    connection->send_handler = send_handler;
    connection->recv_handler = recv_handler;
    connection->destroy_handler = destroy_handler;
    connection->context = create_client_context(listening->context);
    connection->check_status = check_status;
    return SS_TRUE;
}

static void recv_handler(ss_connection_t *connection)
{
    client_receive_function(connection->fd, (client_context_t *)connection->context);
}

static void send_handler(ss_connection_t *connection)
{
    client_send_function(connection->fd, (client_context_t *)connection->context);
}

static void destroy_handler(ss_connection_t *connection)
{
    release_client_context((client_context_t *)connection->context);
}

static client_context_t *create_client_context(void *server)
{
    client_context_t *context = malloc(sizeof(client_context_t));
    ss_ring_buffer_initialize(&context->buffer, 1024);
    context->nbytes_sent = 0;
    context->dead = SS_FALSE;
    ss_testing_session_initialize(&context->session, (ss_testing_server_t *)server);
    return context;
}

static void release_client_context(client_context_t *context)
{
    ss_testing_session_uninitialize(&context->session);
    ss_ring_buffer_uninitialize(&context->buffer);
    free(context);
}

static void client_receive_function(int connfd, client_context_t *context)
{
    ssize_t      nbytes_recv;
    char         buffer[1024], arg[1024], *newarg;
    size_t       cli_length, arg_length;
    size_t       i;

    ss_ring_buffer_recv(connfd, &context->buffer, ss_ring_buffer_free_size(&context->buffer));
    if (!context->session.finished) return;
    if (context->session.action == SS_TESTING_ACTION_QUIT) {
        context->dead = SS_TRUE;
        return;
    }
    if (context->session.response_buffer.length != 0) return;
    do {
        for (cli_length = 0; cli_length < context->buffer.length; cli_length++) {
            ss_ring_buffer_steal((void *)buffer + cli_length, &context->buffer, cli_length, 1);
            if (buffer[cli_length] == '\r' || buffer[cli_length] == '\n') {
                ss_ring_buffer_read(SS_NULL, &context->buffer, cli_length + 1);
                break;
            }
        }
    } while (cli_length == 0 && context->buffer.length != 0);
    if (cli_length == 0) return;
    ss_testing_session_reset(&context->session);
    arg_length = 0;
    for (i = 0; i < cli_length; i++) {
        switch (buffer[i]) {
            case ' ':
            case '\t':
                if (arg_length > 0) {
                    newarg = malloc(arg_length + 1);
                    memcpy(newarg, arg, arg_length);
                    newarg[arg_length] = '\0';
                    context->session.argv[context->session.argc++] = newarg;
                    arg_length = 0;
                }
                break;
            default:
                arg[arg_length++] = buffer[i];
                break;
        }
    }

    if (arg_length > 0) {
        newarg = malloc(arg_length + 1);
        memcpy(newarg, arg, arg_length);
        newarg[arg_length] = '\0';
        context->session.argv[context->session.argc++] = newarg;
        arg_length = 0;
    }
    if (context->session.argc > 0)
        ss_testing_command(&context->session);
}

static void client_send_function(int connfd, client_context_t *context)
{
    ss_testing_poll(&context->session);
    if (context->session.response_buffer.length != 0) {
        ss_ring_buffer_send(connfd, &context->session.response_buffer, context->session.response_buffer.length);
    }
}

static ss_bool_t check_status(ss_connection_t *connection)
{
    client_context_t *context;
    context = (client_context_t *)connection->context;
    if (context->session.finished) {
        if (context->session.action == SS_TESTING_ACTION_QUIT)
            context->dead = SS_TRUE;
    }
    return !context->dead || context->session.response_buffer.length != 0;
}
