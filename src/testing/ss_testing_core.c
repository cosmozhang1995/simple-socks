#include "testing/ss_testing_core.h"
#include "testing/ss_testing_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/ss_strmap.h"
#include "util/ss_ring_buffer.h"

#include "testing/ss_testing_echo.h"
#include "testing/ss_testing_quit.h"

static ss_strmap_t command_map;

void ss_testing_start()
{
    ss_strmap_initialize(&command_map);
    ss_testing_register_command(SS_TESTING_COMMAND_QUIT, ss_testing_comamnd_quit);
    ss_testing_register_command(SS_TESTING_COMMAND_ECHO, ss_testing_comamnd_echo);
}

void ss_testing_stop()
{
    ss_strmap_uninitialize(&command_map);
}

static void default_command(ss_testing_session_t *session);

void ss_testing_command(ss_testing_session_t *session)
{
    ss_variable_t           var;
    ss_testing_command_t    function;
    
    if (ss_strmap_get(&command_map, session->argv[0], &var))
        function = (ss_testing_command_t)var.ptr;
    else
        function = default_command;
    
    function(session);
}

void ss_testing_register_command(const char *name, ss_testing_command_t command)
{
    ss_variable_t var;
    var.ptr = command;
    ss_strmap_put(&command_map, name, var, 0);
}

static void default_command(ss_testing_session_t *session)
{
    char resp[1024];
    sprintf(resp, "command \"%s\" is not defined.\n\r", session->argv[0]);
    ss_ring_buffer_write(&session->response_buffer, resp, strlen(resp));
    session->finished = SS_TRUE;
}

void ss_testing_session_initialize(ss_testing_session_t *session)
{
    ss_ring_buffer_initialize(&session->response_buffer, 1024);
    ss_testing_session_reset(session);
}

void ss_testing_session_reset(ss_testing_session_t *session)
{
    size_t i;
    session->finished = SS_TRUE;
    for (i = 0; i < session->argc; i++) {
        if (session->argv[i]) {
            free(session->argv[i]);
            session->argv[i] = SS_NULL;
        }
    }
    session->argc = 0;
    session->action = 0;
    ss_ring_buffer_reset(&session->response_buffer);
}

void ss_testing_session_uninitialize(ss_testing_session_t *session)
{
    ss_testing_session_reset(session);
    ss_ring_buffer_uninitialize(&session->response_buffer);
}
