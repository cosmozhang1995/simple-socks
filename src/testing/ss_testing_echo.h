#ifndef _SS_TESTING_SS_TESTING_ECHO_H_
#define _SS_TESTING_SS_TESTING_ECHO_H_

#include "testing/ss_testing_core.h"

#define SS_TESTING_COMMAND_ECHO "echo"

void ss_testing_comamnd_echo_processor(ss_testing_session_t *session);

extern const ss_testing_command_t ss_testing_comamnd_echo;

#endif // _SS_TESTING_SS_TESTING_ECHO_H_
