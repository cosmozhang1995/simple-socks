#include "ss_testing_echo.h"

#include <string.h>

#include "util/ss_ring_buffer.h"

const ss_testing_command_t ss_testing_comamnd_echo = {
    SS_NULL,
    SS_NULL,
    ss_testing_comamnd_echo_processor,
    SS_NULL
};

void ss_testing_comamnd_echo_processor(ss_testing_session_t *session)
{
    int         i;
    const char *eol       = "\n\r";
    const char *delimeter = " ";
    for (i = 1; i < session->argc; i++) {
        if (i > 1) ss_ring_buffer_write(&session->response_buffer, delimeter, strlen(delimeter));
        ss_ring_buffer_write(&session->response_buffer, session->argv[i], strlen(session->argv[i]));
    }
    ss_ring_buffer_write(&session->response_buffer, eol, strlen(eol));
    session->finished = SS_TRUE;
}
