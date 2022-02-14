#include "network/ss_connection.h"

#include <stdlib.h>
#include <memory.h>

#include "network/ss_connection_def.h"

void ss_connection_initialize(ss_connection_t *connection)
{
    memset(connection, 0, sizeof(ss_connection_t));
    connection->fd = -1;
}

ss_connection_t *ss_connection_create()
{
    ss_connection_t *connection;
    connection = malloc(sizeof(ss_connection_t));
    ss_connection_initialize(connection);
    return connection;
}

void ss_connection_uninitialize(ss_connection_t *connection)
{
    if (connection->destroy_handler)
        connection->destroy_handler(connection);
}

void ss_connection_release(ss_connection_t *connection)
{
    free(connection);
}
