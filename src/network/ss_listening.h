#ifndef _SS_NETWORK_SS_LISTENING_H_
#define _SS_NETWORK_SS_LISTENING_H_

#include <netinet/in.h>

#include "common/ss_types.h"
#include "network/ss_inet.h"
#include "network/ss_listening_def.h"

ss_listening_t *ss_network_listen(int type, ss_addr_t addr);

#endif // _SS_NETWORK_SS_LISTENING_H_
