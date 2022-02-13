#ifndef __SS_AUTH_PLAIN_CONTEXT_H__
#define __SS_AUTH_PLAIN_CONTEXT_H__

#include "common/ss_types.h"

typedef struct {
} ss_auth_plain_context_t;

void ss_auth_plain_context_initialize(ss_auth_plain_context_t *context);
void ss_auth_plain_context_uninitialize(ss_auth_plain_context_t *context);

#endif
