#include <stdio.h>
#include "receive_only_server.h"
#include "simple_socks5_server.h"

int main(int argc, char *argv[]) {
    int retcode = simple_socks5_server();
    return retcode;
}