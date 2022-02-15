#include <stdio.h>
#include "network/ss_network.h"
#include "server/simple_server.h"
#include "server/socks5_server.h"

int main(int argc, char *argv[]) {
    int rc;
    if ((rc = ss_network_prepare()) != 0) {
        printf("failed to initialize network.\n");
        goto _l_end;
    }
    if ((rc = simple_server_start()) != 0) {
        printf("failed to start server.\n");
        goto _l_end;
    }
    if ((rc = socks5_start(argc, argv)) != 0) {
        printf("failed to start server.\n");
        goto _l_end;
    }
    ss_network_main_loop();
    if ((rc = ss_network_stop()) != 0) {
        printf("failed to stop network.\n");
        goto _l_end;
    }
_l_end:
    return rc;
}