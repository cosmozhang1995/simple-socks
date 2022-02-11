#include <stdio.h>
#include "server/simple_server.h"
// #include "simple_socks5_server.h"

int main(int argc, char *argv[]) {
    int retcode;
    retcode = simple_server();
    return retcode;
}