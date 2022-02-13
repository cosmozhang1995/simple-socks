#include "socks5/ss_context.h"

#include "socks5/ss_auth_method.h"

void ss_context_initialize(ss_context_t *context, ss_config_t *config)
{
    context->status = SS_STATUS_NEGOTIATING;
    context->config = config;
    ss_ring_buffer_initialize(&context->read_buffer, 1024);
    ss_ring_buffer_initialize(&context->write_buffer, 1024);
    context->auth_method = SOCKS5_AUTH_NO_ACCEPTABLE;
    ss_auth_context_initialize(&context->auth);
}

void ss_context_uninitialize(ss_context_t *context)
{
    ss_ring_buffer_uninitialize(&context->read_buffer);
    ss_ring_buffer_uninitialize(&context->write_buffer);
    ss_auth_context_uninitialize(&context->auth);
}
