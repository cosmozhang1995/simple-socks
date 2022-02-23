#include "util/ss_token_pool.h"

#include <memory.h>
#include <stdlib.h>

ss_token_pool_t *ss_token_pool_create()
{
    ss_token_pool_t *instance = malloc(sizeof(ss_token_pool_t));
    ss_token_pool_initialize(instance);
    return instance;
}

void ss_token_pool_destroy(ss_token_pool_t *instance)
{
    ss_token_pool_uninitialize(instance);
    free(instance);
}

void ss_token_pool_initialize(ss_token_pool_t *instance)
{
    memset(instance, 0, sizeof(ss_token_pool_t));
}

void ss_token_pool_uninitialize(ss_token_pool_t *instance)
{
}

