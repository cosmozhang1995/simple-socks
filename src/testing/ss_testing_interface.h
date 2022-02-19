#ifndef _SS_TESTING_SS_TESTING_INTERFACE_H_
#define _SS_TESTING_SS_TESTING_INTERFACE_H_

#include "common/ss_types.h"
#include "util/ss_ring_buffer.h"

#define SS_TESTING_ACTION_QUIT 1

#define SS_TESTING_MAX_CONTEXTS 4

typedef struct ss_testing_server_s ss_testing_server_t;
typedef struct ss_testing_session_s ss_testing_session_t;

struct ss_testing_server_s {
};

struct ss_testing_session_s {
    ss_bool_t            finished;
    int                  argc;
    char                *argv[32];
    ss_ring_buffer_t     response_buffer;
    ss_uint8_t           action;
    ss_testing_server_t *server;
    void                *context[SS_TESTING_MAX_CONTEXTS];
};

ss_testing_server_t *ss_testing_start();
void ss_testing_command(ss_testing_session_t *session);
void ss_testing_poll(ss_testing_session_t *session);
void ss_testing_stop(ss_testing_server_t *server);

void ss_testing_session_initialize(ss_testing_session_t *session, ss_testing_server_t *server);
void ss_testing_session_reset(ss_testing_session_t *session);
void ss_testing_session_uninitialize(ss_testing_session_t *session);

#endif // _SS_TESTING_SS_TESTING_INTERFACE_H_
