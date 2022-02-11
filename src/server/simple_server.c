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

typedef struct {
    size_t buffer_offset;
    size_t buffer_length;
    size_t buffer_capacity;
    char buffer[1024];
    size_t nbytes_sent;
} client_context_t;

static client_context_t *create_client_context();
static void release_client_context(client_context_t *context);
static void client_receive_function(int connfd, client_context_t *context);
static void client_send_function(int connfd, client_context_t *context);
static void client_send_hexcode_function(int connfd, client_context_t *context);

int simple_server() {
    server_config_t config;
    config.create_client_context_function = (create_client_context_function_t)create_client_context;
    config.release_client_context_function = (release_client_context_function_t)release_client_context;
    config.client_recv_handler = (client_recv_function_t)client_receive_function;
    config.client_send_handler = (client_send_function_t)client_send_hexcode_function;
    return server(config);
}

static client_context_t *create_client_context()
{
    client_context_t *context = malloc(sizeof(client_context_t));
    context->buffer_offset = 0;
    context->buffer_length = 0;
    context->buffer_capacity = sizeof(context->buffer);
    context->nbytes_sent = 0;
    return context;
}

static void release_client_context(client_context_t *context)
{
    free(context);
}

static int handle_recv_result(ssize_t result, size_t desired_recv_bytes, client_context_t *context) {
    int recv_errno;
    if (result == 0) {
        return 0;
    }
    if (result < 0) {
        switch (recv_errno = errno)
        {
        case EAGAIN:
            // no more data to read
            break;
        default:
            printf("recv got error %d\n", recv_errno);
            break;
        }
        return 0;
    }
    context->buffer_length += result;
    if (result < desired_recv_bytes) {
        return 0;
    } else {
        return 1;
    }
}

static void client_receive_function(int connfd, client_context_t *context)
{
    unsigned int line_units = 0;
    ssize_t recv_ret;
    size_t nbytes_to_read;
    ssize_t i;
    int recv_errno;

    if (context->buffer_offset + context->buffer_length < context->buffer_capacity) {
        nbytes_to_read = context->buffer_capacity - context->buffer_offset - context->buffer_length;
        recv_ret = recv(connfd, context->buffer + context->buffer_offset, nbytes_to_read, MSG_DONTWAIT);
        if (!handle_recv_result(recv_ret, nbytes_to_read, context))
            return;
    }
    if ((context->buffer_offset + context->buffer_length) % context->buffer_capacity < context->buffer_offset) {
        nbytes_to_read = context->buffer_offset - (context->buffer_offset + context->buffer_length) % context->buffer_capacity;
        recv_ret = recv(connfd, context->buffer + context->buffer_offset, nbytes_to_read, MSG_DONTWAIT);
        if (!handle_recv_result(recv_ret, nbytes_to_read, context))
            return;
    }
}

#define CHUNK_SIZE 16
#define MAX(x, y) ((x) < (y) ? y : x)
#define MIN(x, y) ((x) < (y) ? x : y)

static void client_send_function(int connfd, client_context_t *context)
{
    size_t nbytes_to_write;
    ssize_t nbytes_written;
    while (context->buffer_length >= 0) {
        nbytes_to_write = MIN(context->buffer_length, CHUNK_SIZE);
        if (context->buffer_offset + nbytes_to_write > context->buffer_capacity)
            nbytes_to_write = context->buffer_capacity - context->buffer_offset;
        nbytes_written = send(connfd, (void*)(context->buffer + context->buffer_offset), nbytes_to_write, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (nbytes_written <= 0) {
            break;
        }
        context->buffer_offset = (context->buffer_offset + (size_t)nbytes_written) % context->buffer_capacity;
        context->buffer_length -= nbytes_written;
    }
}

#define SCREEN_WIDTH 8
static void client_send_hexcode_function(int connfd, client_context_t *context)
{
    size_t nbytes_to_write;
    ssize_t nbytes_written;
    char buffer[SCREEN_WIDTH * 3 + 1];
    size_t buffer_offset = 0;
    while (context->buffer_length > 0) {
        sprintf(buffer + buffer_offset, "%02x", (unsigned char)context->buffer[context->buffer_offset]);
        buffer[buffer_offset + 2] = ' ';
        if (++context->buffer_offset == context->buffer_capacity) context->buffer_offset = 0;
        context->buffer_length--;
        buffer_offset += 3;
        if ((++context->nbytes_sent) % SCREEN_WIDTH == 0) {
            buffer[buffer_offset++] = '\n';
            buffer[buffer_offset++] = '\r';
            nbytes_written = send(connfd, (void*)buffer, buffer_offset, MSG_DONTWAIT | MSG_NOSIGNAL);
            buffer_offset = 0;
            if (nbytes_written < 0) break;
        }
    }
    if (buffer_offset > 0) {
        buffer[buffer_offset++] = '\n';
        buffer[buffer_offset++] = '\r';
        send(connfd, (void*)buffer, buffer_offset, MSG_DONTWAIT | MSG_NOSIGNAL);
        buffer_offset = 0;
    }
}