#ifndef _SS_TESTING_SS_TESTING_DNS_H_
#define _SS_TESTING_SS_TESTING_DNS_H_

#include "testing/ss_testing_core.h"

#define SS_TESTING_COMMAND_DNS "dns"

void ss_testing_command_dns_initialize(ss_testing_server_wrapper_t *server);
void ss_testing_command_dns_uninitialize(ss_testing_server_wrapper_t *server);
void ss_testing_comamnd_dns_porcessor(ss_testing_session_t *session);
void ss_testing_comamnd_dns_poll(ss_testing_session_t *session);

extern const ss_testing_command_t ss_testing_command_dns;

#endif // _SS_TESTING_SS_TESTING_DNS_H_
