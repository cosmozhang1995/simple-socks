#include "socks5/ss_auth_plain.h"

#include <string.h>

#include "socks5/ss_server_helper.h"
#include "socks5/ss_process_def.h"
#include "socks5/ss_auth_plain_context.h"
#include "util/ss_strmap.h"
#include "util/ss_io_helper.h"

#define SS_AUTH_PLAIN_VERSION        0x01

#define SS_AUTH_PLAIN_STATUS_SUCCESS 0x00
#define SS_AUTH_PLAIN_STATUS_FAILED  0xFF

typedef struct {
    ss_uint8_t version;
    ss_uint8_t status;
} ss_auth_plain_response_t;

ss_int8_t ss_auth_plain_process(int fd, ss_context_t *context)
{
    ss_auth_plain_context_t *ctx;
    size_t offset;
    ss_uint8_t version, ulen, plen;
    char username[256], password[256];
    ss_variable_t temp_var;
    ss_auth_plain_response_t resp;
   
    ctx = &context->auth.context.plain;
    if (!ss_recv_via_buffer((void*)&version, fd, &context->read_buffer, offset, sizeof(version)))
        goto _l_again;
    if (version != SS_AUTH_PLAIN_VERSION)
        goto _l_error;
    if (!ss_recv_via_buffer((void*)&ulen, fd, &context->read_buffer, offset, sizeof(ulen)))
        goto _l_again;
    if (!ss_recv_via_buffer((void*)username, fd, &context->read_buffer, offset, ulen))
        goto _l_again;
    username[(size_t)ulen] = '\0';
    if (!ss_recv_via_buffer((void*)&plen, fd, &context->read_buffer, offset, sizeof(plen)))
        goto _l_again;
    if (!ss_recv_via_buffer((void*)password, fd, &context->read_buffer, offset, plen))
        goto _l_again;
    password[(size_t)plen] = '\0';

    resp.version = SS_AUTH_PLAIN_VERSION;

    if (!ss_strmap_get(&context->config->auth_plain.map, username, &temp_var))
        goto _l_failed;
    if (strcmp(temp_var.ptr, password) != 0)
        goto _l_failed;

_l_success:
    resp.status = SS_AUTH_PLAIN_STATUS_SUCCESS;
    ss_buffered_write(context, &resp, sizeof(resp));
    return SS_PROCESS_DONE;
    
_l_failed:
    resp.status = SS_AUTH_PLAIN_STATUS_FAILED;
    ss_buffered_write(context, &resp, sizeof(resp));
_l_error:
    return SS_PROCESS_ERROR;

_l_again:
    return SS_PROCESS_AGAIN;
}
