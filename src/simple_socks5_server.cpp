#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

int simple_socks5_server() {
    sockaddr_in ipv4_addr = { AF_INET, htons(1080), inet_addr("0.0.0.0") };
    int retcode = 0;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse_flag = 1;
    if (sockfd < 0) {
        printf("failed to create the socket.\n");
        goto _l_exit;
    }

#define CHECK_CALL(statement, error_message) \
    if ((retcode = (statement)) != 0) { \
        printf(error_message " ERROR [%d]\n", errno); \
        goto _l_exit; \
    }

    CHECK_CALL(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse_flag, sizeof(reuse_flag)), "reuse addr failed");
    CHECK_CALL(bind(sockfd, (sockaddr*)&ipv4_addr, sizeof(ipv4_addr)), "binding failed");
    CHECK_CALL(listen(sockfd, 4), "listen failed");
    char buffer[BUFFER_SIZE];
    printf("listening...\n");
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int connfd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
        if (connfd < 0) {
            printf("failed to establish connection. ERROR [%d]\n", errno);
            continue;
        }
        printf("* connection established\n");
        unsigned int line_units = 0;
        while (true) {
            ssize_t nbytes = read(connfd, buffer, BUFFER_SIZE);
            if (nbytes > 0) {
                // fwrite((void*)buffer, 1, nbytes, stdout);
                for (ssize_t i = 0; i < nbytes; i++) {
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
            else if (nbytes < 0) {
                EAGAIN;
                printf("\n* connection closed\n\n");
                close(connfd);
                break;
            }
        }
    }
_l_exit:
    if (sockfd >= 0) {
        close(sockfd);
    }
    return retcode;
}