#include "server/simple_server.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "server/server.h"
#include "util/ss_ring_buffer.h"

typedef struct {
    ss_ring_buffer_t buffer;
    size_t nbytes_sent;
    int dirty;
} client_context_t;

static client_context_t *create_client_context(void *server_config);
static void release_client_context(client_context_t *context);
static void client_receive_function(int connfd, client_context_t *context);
static void client_send_function(int connfd, client_context_t *context);
static void client_send_hexcode_function(int connfd, client_context_t *context);

int simple_server() {
    server_config_t config;
    config.create_client_context_function = (ssbs_create_client_context_function_t)create_client_context;
    config.release_client_context_function = (ssbs_release_client_context_function_t)release_client_context;
    config.client_recv_handler = (ssbs_client_recv_function_t)client_receive_function;
    config.client_send_handler = (ssbs_client_send_function_t)client_send_hexcode_function;
    config.check_client_status_function = SS_NULL;
    return ss_basic_server(config);
}

static client_context_t *create_client_context(void *server_config)
{
    client_context_t *context = malloc(sizeof(client_context_t));
    ss_ring_buffer_initialize(&context->buffer, 1024);
    context->nbytes_sent = 0;
    context->dirty = 0;
    return context;
}

static void release_client_context(client_context_t *context)
{
    ss_ring_buffer_uninitialize(&context->buffer);
    free(context);
}

static void client_receive_function(int connfd, client_context_t *context)
{
    ssize_t nbytes_recv;
    nbytes_recv = ss_ring_buffer_recv(connfd, &context->buffer, ss_ring_buffer_free_size(&context->buffer));
    if (nbytes_recv > 0) context->dirty = 1;
}

static void client_send_function(int connfd, client_context_t *context)
{
    ss_ring_buffer_send(connfd, &context->buffer, context->buffer.length);
}

#define SCREEN_WIDTH 8
static void client_send_hexcode_function(int connfd, client_context_t *context)
{
    size_t nbytes_to_write;
    ssize_t nbytes_written;
    char buffer[SCREEN_WIDTH * 3 + 1];
    size_t buffer_offset = 0;
    unsigned char ch;
    size_t i;
    if (!context->dirty) return;
    context->dirty = 0;
    // while (ss_ring_buffer_read((void*)&ch, &context->buffer, 1) > 0) {
    for (i = 0; i < context->buffer.length; i++) {
        // ss_ring_buffer_steal((void*)&ch, &context->buffer, i, 1);
        ch = ((unsigned char*)context->buffer.buffer)[i];
        sprintf(buffer + buffer_offset, "%02x", ch);
        buffer[buffer_offset + 2] = ' ';
        buffer_offset += 3;
        if ((++context->nbytes_sent) % SCREEN_WIDTH == 0) {
            buffer[buffer_offset++] = '\n';
            buffer[buffer_offset++] = '\r';
            nbytes_written = send(connfd, (void*)buffer, buffer_offset, MSG_NOSIGNAL);
            buffer_offset = 0;
            if (nbytes_written < 0) break;
        }
    }
    if (buffer_offset > 0) {
        buffer[buffer_offset++] = '\n';
        buffer[buffer_offset++] = '\r';
        send(connfd, (void*)buffer, buffer_offset, MSG_NOSIGNAL);
        buffer_offset = 0;
    }
}