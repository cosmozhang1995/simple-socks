#include <stdio.h>
#include "receive_only_server.h"

int main(int argc, char *argv[]) {
    int retcode = receive_only_server();
    return retcode;
}