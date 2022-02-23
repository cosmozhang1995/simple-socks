#ifndef _SS_NETWORK_SS_CALLBACK_H_
#define _SS_NETWORK_SS_CALLBACK_H_

#include "common/ss_types.h"
#include "common/ss_variable.h"
#include "common/ss_time.h"

typedef struct ss_callback_context_s ss_callback_context_t;

typedef void (*ss_callback_handler_t) (ss_callback_context_t *context);

typedef ss_uint8_t ss_callback_status_t;

/**
 * Callback status INIT
 * This callback is waiting for triggering.
 * INIT is always the very first status of a callback. And once the callback exit
 * INIT status, it can never get back to it.
 */
#define SS_CALLBACK_INIT          0

/**
 * Callback status SUCCESS
 * This status indicates that the callback result is success and the data has been
 * properly set.
 */
#define SS_CALLBACK_SUCCESS       1

/**
 * Callback status FAILED
 * This status indicates that the callback result is failed. The data might been
 * set according to the specific case.
 */
#define SS_CALLBACK_FAILED        2

/**
 * Callback status TIMEOUT
 * This status indicates that the callback has expired its TTL. No more operations
 * should be taken on this callback. The data is assumed to be unset.
 */
#define SS_CALLBACK_TIMEOUT       3

struct ss_callback_context_s {
    ss_callback_handler_t    handler;
    ss_callback_status_t     status;
    ss_time_t                expire;
    ss_variable_t            data;
};

void ss_callback_register();

#endif // _SS_NETWORK_SS_CALLBACK_H_
