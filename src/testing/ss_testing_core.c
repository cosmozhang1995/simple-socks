#include "testing/ss_testing_core.h"
#include "testing/ss_testing_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/ss_strmap.h"
#include "util/ss_ring_buffer.h"

#include "testing/ss_testing_echo.h"
#include "testing/ss_testing_quit.h"
#include "testing/ss_testing_dns.h"

static void default_command_processor(ss_testing_session_t *session);

static const ss_testing_command_t default_command = {
    SS_NULL,
    SS_NULL,
    default_command_processor,
    SS_NULL
};

void ss_testing_register_commands(ss_testing_server_wrapper_t *server);
void ss_testing_unregister_commands(ss_testing_server_wrapper_t *server);

ss_testing_server_t *ss_testing_start()
{
    ss_testing_server_wrapper_t *server;
    server = malloc(sizeof(ss_testing_server_wrapper_t));
    memset(server, 0, sizeof(ss_testing_server_wrapper_t));
    ss_strmap_initialize(&server->command_map);
    ss_testing_register_commands(server);
    return (ss_testing_server_t *)server;
}

void ss_testing_stop(ss_testing_server_t *server)
{
    ss_testing_unregister_commands((ss_testing_server_wrapper_t *)server);
    ss_strmap_uninitialize(&((ss_testing_server_wrapper_t *)server)->command_map);
    free(server);
}

void ss_testing_command(ss_testing_session_t *session)
{
    ss_variable_t                     var;
    const ss_testing_command_t       *cmd;
    ss_testing_command_processor_t    function;

    if (ss_strmap_get(&(SS_TESTING_GET_SERVER(session)->command_map), session->argv[0], &var))
        cmd = (ss_testing_command_t *)var.ptr;
    else
        cmd = &default_command;
    function = cmd->processor;
    if (function) function(session);
}

void ss_testing_poll_command_visitor(const char *name, ss_variable_t *var, ss_testing_session_t *session)
{
    ss_testing_command_t *cmd;
    cmd = (ss_testing_command_t *)var->ptr;
    if (cmd->poll) {
        cmd->poll(session);
    }
}

void ss_testing_poll(ss_testing_session_t *session)
{
    ss_strmap_foreach(&SS_TESTING_GET_SERVER(session)->command_map,
        (ss_strmap_visitor_t)ss_testing_poll_command_visitor, session);
}

void ss_testing_register_command(ss_testing_server_wrapper_t *server, const char *name, ss_testing_command_t command)
{
    ss_variable_t           var;
    var.ptr = malloc(sizeof(ss_testing_command_t));
    memcpy(var.ptr, &command, sizeof(ss_testing_command_t));
    ss_strmap_put(&server->command_map, name, var, 0);
    if (command.initializer) {
        command.initializer(server);
    }
}

void ss_testing_unregister_command(ss_testing_server_wrapper_t *server, const char *name)
{
    ss_variable_t         var;
    ss_testing_command_t *cmd;
    if (ss_strmap_erase(&server->command_map, name, &var)) {
        cmd = (ss_testing_command_t *)var.ptr;
        if (cmd->uninitializer) {
            cmd->uninitializer(server);
        }
    }
}

void ss_testing_release_command_visitor(const char *name, ss_variable_t *var, ss_testing_server_wrapper_t *server)
{
    ss_testing_command_t *cmd;
    cmd = (ss_testing_command_t *)var->ptr;
    if (cmd->uninitializer) {
        cmd->uninitializer(server);
    }
    free(cmd);
}

void ss_testing_register_commands(ss_testing_server_wrapper_t *server)
{
    ss_testing_register_command(server, SS_TESTING_COMMAND_QUIT, ss_testing_comamnd_quit);
    ss_testing_register_command(server, SS_TESTING_COMMAND_ECHO, ss_testing_comamnd_echo);
    ss_testing_register_command(server, SS_TESTING_COMMAND_DNS, ss_testing_command_dns);
}

void ss_testing_unregister_commands(ss_testing_server_wrapper_t *server)
{
    ss_strmap_foreach(&server->command_map, (ss_strmap_visitor_t)ss_testing_release_command_visitor, server);
    ss_strmap_clear(&server->command_map, SS_NULL, SS_NULL);
}

static void default_command_processor(ss_testing_session_t *session)
{
    char resp[1024];
    sprintf(resp, "command \"%s\" is not defined.\n\r", session->argv[0]);
    ss_ring_buffer_write(&session->response_buffer, resp, strlen(resp));
    session->finished = SS_TRUE;
}

void ss_testing_session_initialize(ss_testing_session_t *session, ss_testing_server_t *server)
{
    memset(session, 0, sizeof(ss_testing_session_t));
    session->server = server;
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
