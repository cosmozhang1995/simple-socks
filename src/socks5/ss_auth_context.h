#ifndef __SS_AUTH_CONTEXT_H__
#define __SS_AUTH_CONTEXT_H__

#include "common/ss_types.h"
#include "socks5/ss_auth_plain_context.h"

typedef struct {
    ss_uint8_t method;
    union {
        ss_auth_plain_context_t plain;
    } context;
} ss_auth_context_t;

void ss_auth_context_initialize(ss_auth_context_t *context);
void ss_auth_context_uninitialize(ss_auth_context_t *context);

#endif
