#ifndef _SS_UTIL_SS_IO_HELPER_H_
#define _SS_UTIL_SS_IO_HELPER_H_

#include "common/ss_types.h"
#include "util/ss_ring_buffer.h"

typedef ss_int8_t ss_io_err_t;

#define SS_IO_OK            0
#define SS_IO_EAGAIN        1
#define SS_IO_ERROR        -1
#define SS_IO_EOVERFLOW    -2

ss_io_err_t ss_recv_via_buffer(void *dest, int fd, ss_ring_buffer_t *buffer, size_t offset, size_t size);
ss_io_err_t ss_recv_via_buffer_auto_inc(void *dest, int fd, ss_ring_buffer_t *buffer, size_t *offset, size_t size);
ss_io_err_t ss_send_via_buffer(ss_ring_buffer_t *buffer, void *src, size_t size);

const char *ss_translate_io_err(ss_io_err_t err);

#endif // _SS_UTIL_SS_IO_HELPER_H_
