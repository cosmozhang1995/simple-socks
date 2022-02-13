#ifndef _SS_REQUESTING_CONTEXT_H_
#define _SS_REQUESTING_CONTEXT_H_

#include "common/ss_types.h"

typedef struct {
    ss_uint8_t status;
    ss_uint8_t version;
    ss_uint8_t command;
} ss_requesting_context_t;

#endif
