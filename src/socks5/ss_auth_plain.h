#ifndef __SS_AUTH_PLAIN_H__
#define __SS_AUTH_PLAIN_H__

#include "common/ss_types.h"
#include "socks5/ss_context.h"

ss_int8_t ss_auth_plain_process(int fd, ss_context_t *context);

#endif
