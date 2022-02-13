#ifndef __SS_NEGOTIATION_H__
#define __SS_NEGOTIATION_H__

#include "common/ss_types.h"
#include "socks5/ss_context.h"

ss_int8_t ss_negotiate_process(int fd, ss_context_t *context);

#endif
