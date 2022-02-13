#ifndef __SS_CONTEXT_H__
#define __SS_CONTEXT_H__

#include "common/ss_types.h"
#include "util/ss_ring_buffer.h"
#include "socks5/ss_config.h"
#include "socks5/ss_auth_context.h"

#define SS_STATUS_INIT                0
#define SS_STATUS_NEGOTIATING         1
#define SS_STATUS_NEGOTIATED          2
#define SS_STATUS_AUTHENTICATING      3
#define SS_STATUS_AUTHENTICATED       4
#define SS_STATUS_REQUESTING          5
#define SS_STATUS_REQUESTED           6
#define SS_STATUS_WORKING             7
#define SS_STATUS_DEAD              255

typedef struct {
    ss_uint8_t status;
    ss_config_t *config;
    ss_ring_buffer_t read_buffer;
    ss_ring_buffer_t write_buffer;
    ss_uint8_t auth_method;
    ss_auth_context_t auth;
} ss_context_t;

void ss_context_initialize(ss_context_t *context, ss_config_t *config);
void ss_context_uninitialize(ss_context_t *context);

#endif
