#include "socks5/ss_server_helper.h"

#include "util/ss_ring_buffer.h"

void ss_client_kill(ss_context_t *context)
{
    context->status = SS_STATUS_DEAD;
}

ssize_t ss_buffered_write(ss_context_t *context, void *data, size_t size)
{
    return ss_ring_buffer_write_fixed(&context->write_buffer, data, size);
}
