#include "socks5/ss_auth_context.h"

#include "socks5/ss_auth_method.h"
#include "socks5/ss_auth_plain_context.h"

void ss_auth_context_initialize(ss_auth_context_t *context)
{
    context->method = SOCKS5_AUTH_NO_ACCEPTABLE;
}

void ss_auth_context_uninitialize(ss_auth_context_t *context)
{
    switch (context->method) {
    case SOCKS5_AUTH_PLAIN:
        ss_auth_plain_context_uninitialize(&context->context.plain);
        break;
    }
    context->method = SOCKS5_AUTH_NO_ACCEPTABLE;
}