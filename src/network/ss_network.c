#include "network/ss_network.h"

#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>

#include "common/ss_defs.h"
#include "util/ss_linked_list.h"
#include "network/ss_listening_def.h"
#include "network/ss_listening.h"
#include "network/ss_connection_def.h"
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

void ss_network_handle_connection(uint32_t events, ss_network_item_t *nitem);
void ss_network_handle_listening(uint32_t events, ss_network_item_t *nitem);

void ss_connection_initialize(ss_connection_t *);
void ss_connection_uninitialize(ss_connection_t *);
void ss_listening_initialize(ss_listening_t *);
void ss_listening_uninitialize(ss_listening_t *);

static ss_inline
int ss_network_item_fd(ss_network_item_t *nitem);

static ss_inline
void ss_network_item_close(ss_network_item_t *nitem, int fd);

int ss_network_prepare()
{
    if (epfd < 0) epfd = epoll_create(1024);
    if (epfd < 0) return -1;
    ss_linked_list_initialize(&network_item_list);
}

int ss_network_stop()
{
    ss_linked_node_t      *node;
    ss_network_item_t     *nitem;
    for (
        node = network_item_list.head->next;
        node != network_item_list.tail;
        node = node->next
    ) {
        nitem = (ss_network_item_t *)node->data;
        if (nitem) {
            printf("closing a client session\n");
            ss_network_item_close(nitem, ss_network_item_fd(nitem));
        }
        node->data = SS_NULL;
    }
    ss_linked_list_release(&network_item_list, SS_NULL);
    return 0;
}

void ss_network_main_loop()
{
    int                   i;
    int                   nevents;
    struct epoll_event    epoll_events[EPOLL_SIZE];
    ss_network_item_t    *nitem;

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
                ss_network_handle_connection(epoll_events[i].events, nitem);
                break;
            case NIT_LISTENING:
                ss_network_handle_listening(epoll_events[i].events, nitem);
                break;
            default:
                break;
            }
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
    free(nitem);
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
    ss_network_item_close(nitem, connection->fd);
}

void ss_network_handle_listening(uint32_t events, ss_network_item_t *nitem_listening)
{
    ss_listening_t      *listening;
    ss_connection_t     *connection;
    ss_network_item_t   *nitem;
    struct epoll_event   epevt;
    ss_addr_t            client_addr;
    socklen_t            client_addr_len;
    int                  fd;
    int                  rc;

    listening = &nitem_listening->item.listening;
    client_addr_len = sizeof(client_addr);
    fd = accept(listening->fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (fd < 0) {
        printf("failed to establish connection. ERROR [%d]\n", errno);
        return;
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);

    nitem = ss_network_item_create(NIT_CONNECTION);
    connection = &nitem->item.connection;
    connection->fd = fd;
    connection->type = listening->type;
    connection->address = client_addr;

    epevt.data.ptr = nitem;
    epevt.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;
    rc = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &epevt);
    if (rc != 0) {
        printf("failed to add fd [%d] to epoll. ERROR [%d]\n", fd, errno);
        close(fd);
        ss_network_item_delete(nitem);
        return;
    }
    if (listening->accpet_handler && !listening->accpet_handler(connection)) {
        printf("connection [%d] is rejected.\n", fd);
        close(fd);
        ss_network_item_delete(nitem);
        return;
    }

    nitem->lnode = ss_linked_list_append(&network_item_list, nitem);
}

static ss_inline
int ss_network_item_fd(ss_network_item_t *nitem)
{
    switch (nitem->type) {
    case NIT_CONNECTION:
        return nitem->item.connection.fd;
    case NIT_LISTENING:
        return nitem->item.listening.fd;
    }
    return -1;
}

static ss_inline
void ss_network_item_close(ss_network_item_t *nitem, int fd)
{
    close(fd);
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0) != 0) {
        printf("WARNING: failed to delete fd [%d] from epoll. ERROR [%d]\n", fd, errno);
    }
    ss_linked_list_remove(nitem->lnode);
    ss_network_item_delete(nitem);
}

ss_bool_t ss_network_listen(int type, ss_addr_t addr)
{
    int                  fd;
    ss_listening_t      *listening;
    ss_network_item_t   *nitem;
    struct epoll_event   epevt;
    int                  temp;
    int                  rc;

    fd = socket(addr.domain, type, 0);

    nitem = ss_network_item_create(NIT_LISTENING);
    listening = &nitem->item.listening;
    listening->fd = fd;
    listening->type = type;
    listening->address = addr;

    epevt.data.ptr = nitem;
    epevt.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &epevt);

    if (fd < 0) {
        printf("failed to create the socket.\n");
        goto _l_failed;
    }

    temp = 1;

#define CHECK_CALL(statement, error_message) \
    if ((rc = (statement)) != 0) { \
        printf(error_message " ERROR [%d]\n", errno); \
        goto _l_failed; \
    }

    CHECK_CALL(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&temp, sizeof(temp)), "reuse addr failed");
    CHECK_CALL(bind(fd, (struct sockaddr*)&addr, sizeof(addr)), "binding failed");
    CHECK_CALL(listen(fd, 16), "listen failed");

#undef CHECK_CALL

    return SS_TRUE;

_l_failed:
    close(fd);
    ss_network_item_delete(nitem);
    return SS_FALSE;
}
