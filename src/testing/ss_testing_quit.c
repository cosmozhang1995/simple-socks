#include "testing/ss_testing_quit.h"

#include <stdio.h>

const ss_testing_command_t ss_testing_comamnd_quit = {
    SS_NULL,
    SS_NULL,
    ss_testing_comamnd_quit_processor,
    SS_NULL
};

void ss_testing_comamnd_quit_processor(ss_testing_session_t *session)
{
    session->action = SS_TESTING_ACTION_QUIT;
    session->finished = SS_TRUE;
}
