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

ss_bool_t ss_recv_via_buffer(void *dest, int fd, ss_context_t *context, size_t offset, size_t size)
{
    size_t expected_buffer_size;
    expected_buffer_size = offset + size;
    if (context->read_buffer.length < expected_buffer_size &&
        ss_ring_buffer_recv(fd, &context->read_buffer, expected_buffer_size - context->read_buffer.length) <= 0)
        return SS_FALSE;
    if (dest && (size_t)ss_ring_buffer_steal(dest, &context->read_buffer, offset, size) != size)
        return SS_FALSE;
    return SS_TRUE;
}

ss_bool_t ss_recv_via_buffer_auto_inc(void *dest, int fd, ss_context_t *context, size_t *offset, size_t size)
{
    if (ss_recv_via_buffer(dest, fd, context, *offset, size)) {
        *offset += size;
        return SS_TRUE;
    } else {
        return SS_FALSE;
    }
}
