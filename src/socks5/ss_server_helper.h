#ifndef __SS_SERVER_HELPER_H__
#define __SS_SERVER_HELPER_H__

#include <sys/types.h>
#include "socks5/ss_context.h"

void ss_client_kill(ss_context_t *context);
ssize_t ss_buffered_write(ss_context_t *context, void *data, size_t size);
ss_bool_t ss_recv_via_buffer(void *dest, int fd, ss_context_t *context, size_t offset, size_t size);
ss_bool_t ss_recv_via_buffer_auto_inc(void *dest, int fd, ss_context_t *context, size_t *offset, size_t size);

#endif
