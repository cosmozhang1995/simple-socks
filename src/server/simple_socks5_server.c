// #include "server/simple_socks5_server.h"

// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <errno.h>
// #include "server/server.h"

// #define BUFFER_SIZE 1024
// static char buffer[BUFFER_SIZE];

// static void client_main_loop(int connfd);

// int simple_socks5_server() {
//     server_config_t config;
//     config.client_main_loop = client_main_loop;
//     return server(config);
// }

// static void client_main_loop(int connfd) {
//     unsigned int line_units = 0;
//     ssize_t nbytes_read;
//     char buffer[BUFFER_SIZE];
//     ssize_t i;

//     while (1) {
//         nbytes_read = read(connfd, buffer, BUFFER_SIZE);
//         if (nbytes_read > 0) {
//             // fwrite((void*)buffer, 1, nbytes_read, stdout);
//             for (i = 0; i < nbytes_read; i++) {
//                 fprintf(stdout, "%02x", (unsigned int)buffer[i]);
//                 if (++line_units == 8) {
//                     line_units = 0;
//                     fprintf(stdout, "\n");
//                 } else {
//                     fprintf(stdout, " ");
//                 }
//             }
//             fflush(stdout);
//         }
//         else if (nbytes_read < 0) {
//             printf("\n* connection closed\n\n");
//             break;
//         }
//     }
// }