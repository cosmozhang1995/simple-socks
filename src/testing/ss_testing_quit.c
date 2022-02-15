#include "testing/ss_testing_quit.h"

#include <stdio.h>

void ss_testing_comamnd_quit(ss_testing_session_t *session)
{
    session->action = SS_TESTING_ACTION_QUIT;
    session->finished = SS_TRUE;
}
