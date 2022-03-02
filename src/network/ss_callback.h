#ifndef _SS_NETWORK_SS_CALLBACK_H_
#define _SS_NETWORK_SS_CALLBACK_H_

#include "network/ss_callback_def.h"

ss_callback_context_t *ss_callback_register(ss_callback_handler_t handler, ss_time_t timeout);

#endif // _SS_NETWORK_SS_CALLBACK_H_
