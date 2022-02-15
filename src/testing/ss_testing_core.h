#ifndef _SS_TESTING_SS_TESTING_CORE_H_
#define _SS_TESTING_SS_TESTING_CORE_H_

#include "ss_testing_interface.h"

typedef void (*ss_testing_command_t) (ss_testing_session_t *session);

void ss_testing_register_command(const char *name, ss_testing_command_t command);

#endif // _SS_TESTING_SS_TESTING_CORE_H_
