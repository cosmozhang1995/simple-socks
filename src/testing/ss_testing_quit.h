#ifndef _SS_TESTING_SS_TESTING_QUIT_H_
#define _SS_TESTING_SS_TESTING_QUIT_H_

#include "testing/ss_testing_core.h"

#define SS_TESTING_COMMAND_QUIT "quit"

void ss_testing_comamnd_quit_processor(ss_testing_session_t *session);

extern const ss_testing_command_t ss_testing_comamnd_quit;

#endif // _SS_TESTING_SS_TESTING_QUIT_H_
