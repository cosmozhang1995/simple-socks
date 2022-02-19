#ifndef _SS_TESTING_SS_TESTING_CORE_H_
#define _SS_TESTING_SS_TESTING_CORE_H_

#include "util/ss_strmap_def.h"
#include "testing/ss_testing_interface.h"

typedef struct ss_testing_command_s ss_testing_command_t;
typedef struct ss_testing_server_wrapper_s ss_testing_server_wrapper_t;
typedef struct ss_testing_session_context_s ss_testing_session_context_t;

typedef void (*ss_testing_command_initializer_t) (ss_testing_server_wrapper_t *server);
typedef void (*ss_testing_command_uninitializer_t) (ss_testing_server_wrapper_t *server);
typedef void (*ss_testing_command_processor_t) (ss_testing_session_t *session);
typedef void (*ss_testing_command_poll_t) (ss_testing_session_t *session);

struct ss_testing_command_s {
    ss_testing_command_initializer_t     initializer;
    ss_testing_command_uninitializer_t   uninitializer;
    ss_testing_command_processor_t       processor;
    ss_testing_command_poll_t            poll;
};

struct ss_testing_server_wrapper_s {
    ss_testing_server_t export;
    ss_strmap_t         command_map;
    void               *context[SS_TESTING_MAX_CONTEXTS];
};

#define SS_TESTING_GET_SERVER(session) ((ss_testing_server_wrapper_t *)((session)->server))

#endif // _SS_TESTING_SS_TESTING_CORE_H_
