#ifndef __SS_AUTH_H__
#define __SS_AUTH_H__

#include "common/ss_types.h"
#include "socks5/ss_context.h"
#include "socks5/ss_auth_method.h"
#include "socks5/ss_auth_context.h"
#include "socks5/ss_auth_plain_context.h"
#include "socks5/ss_process_def.h"
#include "socks5/ss_auth_plain.h"

ss_int8_t ss_auth_begin(ss_context_t *context)
{
    ss_auth_context_initialize(&context->auth);
    switch (context->auth_method) {
    case SOCKS5_AUTH_PLAIN:
        ss_auth_plain_context_initialize(&context->auth.context.plain);
        break;
    default:
        return SS_PROCESS_ERROR;
    }
    context->auth.method = context->auth_method;
    return SS_PROCESS_DONE;
}

ss_int8_t ss_auth_process(int fd, ss_context_t *context)
{
    switch (context->auth.method) {
    case SOCKS5_AUTH_PLAIN:
        return ss_auth_plain_process(fd, context);
        break;
    default:
        break;
    }
    return SS_PROCESS_ERROR;
}

ss_int8_t ss_auth_finish(ss_context_t *context)
{
    ss_auth_context_uninitialize(&context->auth);
}

#endif
