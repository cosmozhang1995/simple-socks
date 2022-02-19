#include "util/ss_io_helper.h"

ss_io_err_t ss_recv_via_buffer(void *dest, int fd, ss_ring_buffer_t *buffer, size_t offset, size_t size)
{
    size_t expected_buffer_size;
    if (offset + size > buffer->capacity)
        return SS_IO_EOVERFLOW;
    expected_buffer_size = offset + size;
    if (buffer->length < expected_buffer_size &&
        ss_ring_buffer_recv(fd, buffer, expected_buffer_size - buffer->length) <= 0)
        return SS_IO_EAGAIN;
    if (dest && (size_t)ss_ring_buffer_steal(dest, buffer, offset, size) != size)
        return SS_IO_ERROR;
    return SS_IO_OK;
}

ss_io_err_t ss_recv_via_buffer_auto_inc(void *dest, int fd, ss_ring_buffer_t *buffer, size_t *offset, size_t size)
{
    ss_io_err_t rc;
    if ((rc = ss_recv_via_buffer(dest, fd, buffer, *offset, size)) != SS_IO_OK) {
        return rc;
    }
    *offset += size;
    return SS_IO_OK;
}

ss_io_err_t ss_send_via_buffer(ss_ring_buffer_t *buffer, void *src, size_t size)
{
    if (buffer->capacity < size) {
        return SS_IO_EAGAIN;
    }
    if (ss_ring_buffer_write_fixed(buffer, src, size) != size) {
        return SS_IO_ERROR;
    }
    return SS_IO_OK;
}

const char *ss_translate_io_err(ss_io_err_t err)
{
    switch (err) {
    case SS_IO_OK:
        return "OK";
    case SS_IO_EAGAIN:
        return "EAGAIN";
    case SS_IO_ERROR:
        return "ERROR";
    case SS_IO_EOVERFLOW:
        return "EOVERFLOW";
    default:
        return "<unknown>";
    }
}
