#ifndef _SS_NETWORK_SS_LISTENING_H_
#define _SS_NETWORK_SS_LISTENING_H_

#include "common/ss_types.h"
#include "network/ss_listening_def.h"

// ss_listening_t *ss_listening_create();
// void ss_listening_initialize(ss_listening_t *);
// void ss_listening_uninitialize(ss_listening_t *);
// void ss_listening_release(ss_listening_t *);

ss_bool_t ss_network_listen(ss_listening_t *);

#endif // _SS_NETWORK_SS_LISTENING_H_
