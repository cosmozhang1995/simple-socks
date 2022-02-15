#ifndef _SS_TESTING_SS_TESTING_INTERFACE_H_
#define _SS_TESTING_SS_TESTING_INTERFACE_H_

#include "common/ss_types.h"
#include "util/ss_ring_buffer.h"

#define MAX_ARG_COUNT 32

#define SS_TESTING_ACTION_QUIT 1

typedef struct {
    ss_bool_t            finished;
    int                  argc;
    char                *argv[MAX_ARG_COUNT];
    ss_ring_buffer_t     response_buffer;
    ss_uint8_t           action;
} ss_testing_session_t;

void ss_testing_start();
void ss_testing_command(ss_testing_session_t *session);
void ss_testing_stop();

void ss_testing_session_initialize(ss_testing_session_t *session);
void ss_testing_session_reset(ss_testing_session_t *session);
void ss_testing_session_uninitialize(ss_testing_session_t *session);

#endif // _SS_TESTING_SS_TESTING_INTERFACE_H_
