#ifndef _SS_NETWORK_SS_CALLBACK_SYS_H_
#define _SS_NETWORK_SS_CALLBACK_SYS_H_

#include "common/ss_time.h"

void ss_callback_prepare();
void ss_callback_stop();

/**
 * Check the callback queue. Handle the callback items if necessary.
 * 
 * @return ss_time_t next timeout interval
 */
ss_time_t ss_callback_poll();

#endif // _SS_NETWORK_SS_CALLBACK_SYS_H_