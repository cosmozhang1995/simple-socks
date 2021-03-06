#include "socks5/ss_negotiation.h"

#include "common/ss_types.h"
#include "util/ss_io_helper.h"
#include "socks5/ss_context.h"
#include "socks5/ss_version.h"
#include "socks5/ss_auth_method.h"
#include "socks5/ss_server_helper.h"
#include "socks5/ss_process_def.h"

typedef struct {
    ss_uint8_t version;
    ss_uint8_t method;
} ss_hello_response_t;

ss_int8_t ss_negotiate_process(int fd, ss_context_t *context)
{
    ss_uint8_t             ss_version;
    ss_uint8_t             n_methods;
    ss_uint8_t             method;
    size_t                 offset;
    ss_hello_response_t    resp;
    int                    i;
    ss_io_err_t            rc;

    offset = 0;
    if ((rc = ss_recv_via_buffer_auto_inc((void*)&ss_version, fd, &context->read_buffer, &offset, sizeof(ss_version))) != SS_IO_OK)
        goto _l_error;
    if (ss_version != SOCKS5_VERSION)
        goto _l_error;
    if ((rc = ss_recv_via_buffer_auto_inc((void*)&n_methods, fd, &context->read_buffer, &offset, sizeof(n_methods))) != SS_IO_OK)
        goto _l_error;
    if ((rc = ss_recv_via_buffer(0, fd, &context->read_buffer, offset, (size_t)n_methods)) != SS_IO_OK)
        goto _l_error;

    context->auth_method = SOCKS5_AUTH_NO_ACCEPTABLE;
    while (n_methods-- > 0) {
        ss_ring_buffer_steal((void*)&method, &context->read_buffer, offset++, sizeof(method));
        if (method == context->config->auth_method) {
            context->auth_method = method;
            goto _l_success;
        }
    }

_l_error:
    if (rc == SS_IO_EAGAIN) goto _l_again;
    ss_client_kill(context);
    return SS_PROCESS_ERROR;

_l_success:
    resp.version = SOCKS5_VERSION;
    resp.method = context->auth_method;
    if (ss_send_via_buffer(&context->write_buffer, &resp, sizeof(resp)) != SS_IO_OK) {
        ss_client_kill(context);
        return SS_PROCESS_ERROR;
    }

    ss_ring_buffer_reset(&context->read_buffer);
    context->status = SS_STATUS_NEGOTIATED;
    return SS_PROCESS_DONE;

_l_again:
    return SS_PROCESS_AGAIN;
}