#include "network/ss_listening.h"

#include <stdlib.h>
#include <memory.h>

#include "network/ss_listening_def.h"

void ss_listening_initialize(ss_listening_t *listening)
{
    memset(listening, 0, sizeof(ss_listening_t));
    listening->fd = -1;
}

ss_listening_t *ss_listening_create()
{
    ss_listening_t *listening;
    listening = malloc(sizeof(ss_listening_t));
    ss_listening_initialize(listening);
    return listening;
}

void ss_listening_uninitialize(ss_listening_t *listening)
{
    if (listening->destroy_handler)
        listening->destroy_handler(listening);
}

void ss_listening_release(ss_listening_t *listening)
{
    ss_listening_uninitialize(listening);
    free(listening);
}
