#include "util/ss_io_helper.h"

ss_bool_t ss_recv_via_buffer(void *dest, int fd, ss_ring_buffer_t *buffer, size_t offset, size_t size)
{
    size_t expected_buffer_size;
    expected_buffer_size = offset + size;
    if (buffer->length < expected_buffer_size &&
        ss_ring_buffer_recv(fd, buffer, expected_buffer_size - buffer->length) <= 0)
        return SS_FALSE;
    if (dest && (size_t)ss_ring_buffer_steal(dest, buffer, offset, size) != size)
        return SS_FALSE;
    return SS_TRUE;
}

ss_bool_t ss_recv_via_buffer_auto_inc(void *dest, int fd, ss_ring_buffer_t *buffer, size_t *offset, size_t size)
{
    if (ss_recv_via_buffer(dest, fd, buffer, *offset, size)) {
        *offset += size;
        return SS_TRUE;
    } else {
        return SS_FALSE;
    }
}
