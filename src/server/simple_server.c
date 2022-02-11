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

#define BUFFER_SIZE 1024

typedef struct {
} client_context_t;

static client_context_t *create_client_context();
static void release_client_context(client_context_t *context);
static void client_receive_function(int connfd);

int simple_server() {
    server_config_t config;
    config.client_context_create = (create_client_context_function_t)create_client_context;
    config.client_context_release = (release_client_context_function_t)release_client_context;
    config.client_receive_handler = client_receive_function;
    return server(config);
}

static void client_main_loop(int connfd) {
    unsigned int line_units = 0;
    ssize_t nbytes_read;
    char buffer[BUFFER_SIZE];
    ssize_t i;

    while (1) {
        nbytes_read = read(connfd, buffer, BUFFER_SIZE);
        if (nbytes_read > 0) {
            // fwrite((void*)buffer, 1, nbytes_read, stdout);
            for (i = 0; i < nbytes_read; i++) {
                fprintf(stdout, "%02x", (unsigned int)buffer[i]);
                if (++line_units == 8) {
                    line_units = 0;
                    fprintf(stdout, "\n");
                } else {
                    fprintf(stdout, " ");
                }
            }
            fflush(stdout);
        }
        else if (nbytes_read < 0) {
            printf("\n* connection closed\n\n");
            break;
        }
    }
}

static client_context_t *create_client_context()
{
    client_context_t *context = malloc(sizeof(client_context_t));
    return context;
}

static void release_client_context(client_context_t *context)
{
    free(context);
}

static void client_receive_function(int connfd)
{
    unsigned int line_units = 0;
    ssize_t nbytes_read;
    char buffer[1024];
    ssize_t i;
    int recv_errno;

    while (1) {
        nbytes_read = recv(connfd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (nbytes_read > 0) {
            // fwrite((void*)buffer, 1, nbytes_read, stdout);
            for (i = 0; i < nbytes_read; i++) {
                fprintf(stdout, "%02x", (unsigned int)buffer[i]);
                if (++line_units == 8) {
                    line_units = 0;
                    fprintf(stdout, "\n");
                } else {
                    fprintf(stdout, " ");
                }
            }
            fflush(stdout);
            continue;
        }
        if (nbytes_read == 0) {
            break;
        }
        switch (recv_errno = errno)
        {
        case EAGAIN:
            // no more data to read
            break;
        default:
            printf("recv got error %d\n", recv_errno);
            break;
        }
        break;
    }
}