#ifndef _SS_NETWORK_SS_LISTENING_H_
#define _SS_NETWORK_SS_LISTENING_H_

#include "common/ss_types.h"

typedef struct ss_listening_s ss_listening_t;

typedef ss_int8_t (*ss_listening_accept_handler)          (ss_listening_t *listening);
typedef void      (*ss_listening_release_function_t)      (ss_listening_t *listening);

struct ss_listening_s {
    int                             fd;
    ss_listening_accept_handler     accpet_handler;
    ss_listening_release_function_t release;
};

ss_bool_t ss_network_add_listening(ss_listening_t *);

#endif // _SS_NETWORK_SS_LISTENING_H_
