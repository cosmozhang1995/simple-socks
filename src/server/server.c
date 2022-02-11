#include "server/server.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include "util/linked_list.h"

#define EPOLL_SIZE 1024

typedef struct {
    int fd;
    void *ctx;
    release_client_context_function_t ctx_release_fn;
    linked_node_t *lnode;
} client_session_t;

static int trigger_signal = 0;
static void signal_handler(int signum);

static client_session_t *create_client_session(
    int fd,
    create_client_context_function_t create_client_context_function,
    release_client_context_function_t release_client_context_function);
static void release_client_session(client_session_t *session_ptr);

static int check_server_config(server_config_t config);

int server(server_config_t config) {
    struct sockaddr_in server_addr;
    int retcode = 0;
    int sockfd, epollfd, connfd, tfd;
    int nevents;
    int reuse_flag = 1;
    int i;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct epoll_event sock_epoll_event, client_epoll_event, epoll_events[EPOLL_SIZE];
    client_session_t *client_session_ptr;
    linked_list_t *session_list;
    linked_node_t *session_node;

#define CHECK_CALL(statement, error_message) \
    if ((retcode = (statement)) != 0) { \
        printf(error_message " ERROR [%d]\n", errno); \
        goto _l_exit; \
    }

    CHECK_CALL(check_server_config(config), "illegal server config");

    epollfd = epoll_create(1024);
    session_list = linked_list_create();

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1080);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sock_epoll_event.data.ptr = create_client_session(sockfd, 0, 0);
    sock_epoll_event.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &sock_epoll_event);

    if (sockfd < 0) {
        printf("failed to create the socket.\n");
        goto _l_exit;
    }

    CHECK_CALL(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse_flag, sizeof(reuse_flag)), "reuse addr failed");
    CHECK_CALL(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)), "binding failed");
    CHECK_CALL(listen(sockfd, 4), "listen failed");

    signal(SIGINT, signal_handler);
    printf("sock fd is %d\n", sockfd);
    printf("listening...\n");
    while (1) {
        if (trigger_signal) {
            break;
        }
        nevents = epoll_wait(epollfd, epoll_events, EPOLL_SIZE, 1000);
        if (nevents < 0) {
            break;
        }
        for (i = 0; i < nevents; i++) {
            client_session_ptr = (client_session_t *)epoll_events[i].data.ptr;
            tfd = client_session_ptr->fd;
            if (tfd == sockfd) {
                if (epoll_events[i].events & EPOLLIN) {
                    client_addr_len = sizeof(client_addr);
                    connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
                    if (connfd < 0) {
                        printf("failed to establish connection. ERROR [%d]\n", errno);
                        continue;
                    }
                    // printf("* established connection %d\n", connfd);
                    client_session_ptr = create_client_session(connfd,
                        config.client_context_create, config.client_context_release);
                    client_session_ptr->lnode = linked_list_append(session_list, client_session_ptr);
                    client_epoll_event.data.fd = connfd;
                    client_epoll_event.data.ptr = client_session_ptr;
                    client_epoll_event.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &client_epoll_event);
                }
            } else {
                if (epoll_events[i].events & EPOLLIN) {
                    config.client_receive_handler(tfd);
                }
                if (epoll_events[i].events & EPOLLOUT) {
                }
                if (epoll_events[i].events & EPOLLERR) {
                    printf("client event EPOLLERR\n");
                }
                if (epoll_events[i].events & EPOLLRDHUP) {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, tfd, 0);
                    linked_list_remove(client_session_ptr->lnode);
                    // printf("* closed connection %d\n", tfd);
                }
            }
        }
    }

_l_exit:
    for (
        session_node = session_list->head->next;
        session_node != session_list->tail;
        session_node = session_node->next
    ) {
        client_session_ptr = session_node->data;
        if (client_session_ptr) {
            printf("closing a client session\n");
            close(client_session_ptr->fd);
            release_client_session(client_session_ptr);
        }
        linked_list_remove(session_node);
    }
    linked_list_release(session_list, (release_data_function_t)release_client_session);
    if (sockfd >= 0) {
        printf("closing server\n");
        close(sockfd);
    }
    printf("bye.\n");
    return retcode;
}

static client_session_t *create_client_session(
    int fd,
    create_client_context_function_t create_client_context_function,
    release_client_context_function_t release_client_context_function)
{
    client_session_t *session;
    session = malloc(sizeof(client_session_t));
    session->fd = fd;
    session->ctx = create_client_context_function ? create_client_context_function() : 0;
    session->ctx_release_fn = release_client_context_function;
    session->lnode = 0;
    return session;
}

static void release_client_session(client_session_t *session)
{
    if (session->ctx) {
        if (session->ctx_release_fn) {
            session->ctx_release_fn(session->ctx);
        } else {
            free(session->ctx);
        }
    }
    free(session);
}

static void signal_handler(int signum)
{
    trigger_signal = signum;
}

static int check_server_config(server_config_t config)
{
    if (!config.client_context_create) return -1;
    if (!config.client_context_release) return -1;
    if (!config.client_receive_handler) return -1;
    return 0;
}