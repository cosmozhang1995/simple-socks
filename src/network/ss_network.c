#include "network/ss_network.h"

#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>

#include "common/ss_defs.h"
#include "util/ss_linked_list.h"
#include "network/ss_listening.h"
#include "network/ss_connection.h"

#define EPOLL_SIZE 2048

#define NIT_CONNECTION 1
#define NIT_LISTENING  2

typedef struct {
    ss_uint8_t           type;
    ss_linked_node_t    *lnode;
    union {
        ss_connection_t  connection;
        ss_listening_t   listening;
    }                    item;
} ss_network_item_t;

static ss_linked_list_t network_item_list;

static int trigger_signal = 0;

static int epfd = -1;

int ss_network_prepare()
{
    epfd = epoll_create(1024);
}

void ss_network_handle_connection(uint32_t events, ss_network_item_t *nitem);
void ss_network_handle_listening(uint32_t events, ss_network_item_t *nitem);

void ss_connection_initialize(ss_connection_t *);
void ss_connection_uninitialize(ss_connection_t *);
void ss_listening_initialize(ss_listening_t *);
void ss_listening_uninitialize(ss_listening_t *);

int ss_network_main_loop()
{
    int i;
    int nevents;
    struct epoll_event epoll_events[EPOLL_SIZE];
    ss_network_item_t *nitem;
    epfd = epoll_create(1024);
    while (1) {
        if (trigger_signal != 0) {
            break;
        }
        nevents = epoll_wait(epfd, epoll_events, EPOLL_SIZE, 1000);
        if (nevents < 0) {
            break;
        }
        for (i = 0; i < nevents; i++) {
            nitem = (ss_network_item_t *)epoll_events[i].data.ptr;
            if (!nitem) continue;
            switch (nitem->type) {
            case NIT_CONNECTION:
                ss_network_handle_connection(epoll_events[i].events, &nitem->item.connection);
                break;
            case NIT_LISTENING:
                ss_network_handle_connection(epoll_events[i].events, &nitem->item.connection);
                break;
            default:
                break;
            }
            

            // client_session_ptr = (client_session_t *)epoll_events[i].data.ptr;
            // tfd = client_session_ptr->fd;
            // if (tfd == sockfd) {
            //     if (epoll_events[i].events & EPOLLIN) {
            //         client_addr_len = sizeof(client_addr);
            //         connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
            //         if (connfd < 0) {
            //             printf("failed to establish connection. ERROR [%d]\n", errno);
            //             continue;
            //         }
            //         fcntl(connfd, F_SETFL, O_NONBLOCK);
            //         // printf("* established connection %d\n", connfd);
            //         client_session_ptr = create_client_session(
            //             connfd,
            //             config.server_config,
            //             config.create_client_context_function);
            //         client_epoll_event.data.ptr = client_session_ptr;
            //         client_epoll_event.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;
            //         retcode = epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &client_epoll_event);
            //         if (retcode != 0) {
            //             printf("failed to add fd [%d] to epoll. ERROR [%d]\n", connfd, errno);
            //             close(connfd);
            //             release_client_session(client_session_ptr, config.release_client_context_function);
            //         }
            //         client_session_ptr->lnode = ss_linked_list_append(session_list, client_session_ptr);
            //     }
            // } else {
            //     if (epoll_events[i].events & EPOLLIN) {
            //         if (config.client_recv_handler) {
            //             config.client_recv_handler(tfd, client_session_ptr->ctx);
            //         }
            //     }
            //     if (epoll_events[i].events & EPOLLOUT) {
            //         if (config.client_send_handler) {
            //             config.client_send_handler(tfd, client_session_ptr->ctx);
            //         }
            //     }
            //     if (epoll_events[i].events & EPOLLERR) {
            //         close_client_session(epollfd, client_session_ptr);
            //         continue;
            //     }
            //     if (epoll_events[i].events & EPOLLRDHUP) {
            //         close_client_session(epollfd, client_session_ptr);
            //         continue;
            //     }
            //     if (config.check_client_status_function &&
            //         !config.check_client_status_function(client_session_ptr->ctx))
            //     {
            //         close_client_session(epollfd, client_session_ptr);
            //         continue;
            //     }
            // }
        }
    }
}

void ss_network_item_delete(ss_network_item_t *nitem)
{
    if (!nitem) return;
    switch (nitem->type) {
    case NIT_CONNECTION:
        ss_connection_uninitialize(&nitem->item.connection);
        break;
    case NIT_LISTENING:
        ss_listening_uninitialize(&nitem->item.listening);
        break;
    }
}

int ss_network_stop()
{
}

ss_network_item_t *ss_network_item_create(ss_uint8_t type)
{
    ss_network_item_t *nitem;
    nitem = malloc(sizeof(ss_network_item_t));
    memset((void *)nitem, 0, sizeof(ss_network_item_t));
    nitem->type = type;
    switch (type) {
    case NIT_CONNECTION:
        ss_connection_initialize(&nitem->item.connection);
        break;
    case NIT_LISTENING:
        ss_listening_initialize(&nitem->item.listening);
        break;
    }
    return nitem;
}

void ss_network_item_delete(ss_network_item_t *nitem)
{
    switch (nitem->type) {
    case NIT_CONNECTION:
        ss_connection_uninitialize(&nitem->item.connection);
        break;
    case NIT_LISTENING:
        ss_listening_uninitialize(&nitem->item.listening);
        break;
    }
    free(nitem);
}

// #define SS_NETWORK_ADD_ITEM_FUNCTION(type, item)                                                    \
// ss_bool_t ss_network_add_##item##(ss_##item##_t * item)                                             \
// {                                                                                                   \
//     struct epoll_event   epevt;                                                                     \
//     ss_network_item_t   *nitem;                                                                     \
//     int                  rc;                                                                        \
//                                                                                                     \
//     if (epfd < 0) return SS_FALSE;                                                                  \
//     nitem = ss_network_item_create(type);                                                           \
//     epevt.data.ptr = nitem;                                                                         \
//     epevt.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;                                      \
//     rc = epoll_ctl(epfd, EPOLL_CTL_ADD, item->fd, &epevt);                                          \
//     if (rc != 0) {                                                                                  \
//         printf("failed to add fd [%d] to epoll. ERROR [%d]\n", item->fd, errno);                    \
//         ss_network_item_delete(nitem);                                                              \
//         return SS_FALSE;                                                                            \
//     }                                                                                               \
//     nitem->lnode = ss_linked_list_append(&network_item_list, nitem);                                \
//     nitem->ptr.item = item;                                                                         \
// }

// SS_NETWORK_ADD_ITEM_FUNCTION(NIT_CONNECTION, connection)
// SS_NETWORK_ADD_ITEM_FUNCTION(NIT_LISTENING, listening)

static ss_inline
void ss_network_item_close(ss_network_item_t *nitem, int fd);

void ss_network_handle_connection(uint32_t events, ss_network_item_t *nitem)
{
    ss_connection_t *connection;

    connection = &nitem->item.connection;
    if (events & EPOLLIN) {
        if (connection->recv_handler) {
            connection->recv_handler(connection);
        }
    }
    if (events & EPOLLOUT) {
        if (connection->send_handler) {
            connection->send_handler(connection);
        }
    }
    if (events & (EPOLLERR | EPOLLRDHUP)) {
        goto _l_close;
    }
    if (connection->check_status && !connection->check_status(connection)) {
        goto _l_close;
    }

_l_end:
    return;
_l_close:
    ss_network_item_close(connection->fd, nitem);
}

void ss_network_handle_listening(uint32_t events, ss_network_item_t *nitem)
{
    ss_listening_t      *listening;
    ss_listening_t      *connection;
    ss_network_item_t   *nitem;
    struct epoll_event   epevt;
    struct sockaddr      client_addr;
    size_t               client_addr_len;
    int                  fd;
    int                  rc;

    listening = &nitem->item.listening;
    client_addr_len = sizeof(client_addr);
    fd = accept(listening->fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (fd < 0) {
        printf("failed to establish connection. ERROR [%d]\n", errno);
        return;
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);

    nitem = ss_network_item_create(NIT_CONNECTION);
    connection = &nitem->item.connection;
    connection->fd = fd;
    connection->domain = listening->domain;
    connection->type = listening->domain;
    connection->address = listening->address;

    epevt.data.ptr = nitem;
    epevt.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;
    rc = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &epevt);
    if (rc != 0) {
        printf("failed to add fd [%d] to epoll. ERROR [%d]\n", fd, errno);
        close(fd);
        ss_network_item_delete(connection);
    }
    
    nitem->lnode = ss_linked_list_append(&network_item_list, nitem);

    listening->accpet_handler(connection);
}

static ss_inline
void ss_network_item_close(ss_network_item_t *nitem, int fd)
{
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0) != 0) {
        printf("WARNING: failed to delete fd [%d] from epoll. ERROR [%d]\n", fd, errno);
    }
    ss_linked_list_remove(nitem->lnode);
    ss_network_item_delete(nitem);
}
