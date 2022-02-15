#ifndef _SS_UTIL_SS_IO_HELPER_H_
#define _SS_UTIL_SS_IO_HELPER_H_

#include "common/ss_types.h"
#include "util/ss_ring_buffer.h"

ss_bool_t ss_recv_via_buffer(void *dest, int fd, ss_ring_buffer_t *buffer, size_t offset, size_t size);
ss_bool_t ss_recv_via_buffer_auto_inc(void *dest, int fd, ss_ring_buffer_t *buffer, size_t *offset, size_t size);

#endif // _SS_UTIL_SS_IO_HELPER_H_
