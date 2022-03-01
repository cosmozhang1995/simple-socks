#include "testing/ss_testing_dns.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <time.h>

#include "service/ss_dns.h"
#include "testing/ss_testing_id.h"
#include "util/ss_io_helper.h"

#define SS_TESTING_DNS_GET_CONTEXT(session) ((ss_testing_command_dns_context_t *)(SS_TESTING_GET_SERVER(session)->context[SS_TESTING_CMDID_DNS]))

#define FETCH_TIMEOUT 5

typedef struct ss_testing_command_dns_context_s ss_testing_command_dns_context_t;

const ss_testing_command_t ss_testing_command_dns = {
    ss_testing_command_dns_initialize,
    ss_testing_command_dns_uninitialize,
    ss_testing_comamnd_dns_porcessor,
    ss_testing_comamnd_dns_poll
};

struct ss_testing_command_dns_context_s {
    ss_dns_service_t *service;
    char              current[256];
    ss_time_t         start_fetch_time;
};

void ss_testing_command_dns_initialize(ss_testing_server_wrapper_t *server)
{
    ss_testing_command_dns_context_t *ctx;
    ctx = malloc(sizeof(ss_testing_command_dns_context_t));
    ctx->service = ss_dns_service_start(ss_make_ipv4_addr(getenv("DNS_RESOLVER"), 53));
    server->context[SS_TESTING_CMDID_DNS] = ctx;
}

void ss_testing_command_dns_uninitialize(ss_testing_server_wrapper_t *server)
{
    ss_testing_command_dns_context_t *ctx;
    ctx = (ss_testing_command_dns_context_t *)server->context[SS_TESTING_CMDID_DNS];
    ss_dns_service_stop(ctx->service);
    free(ctx);
}

void ss_testing_comamnd_dns_porcessor(ss_testing_session_t *session)
{
    const char                       *dn;
    ss_testing_command_dns_context_t *ctx;
    char                              msg[1024];
    char                              addrstr[256];
    ss_addr_t                         addr;
    ss_io_err_t                       rc;
    ss_time_t                         now;

    ctx = SS_TESTING_DNS_GET_CONTEXT(session);
    if (ctx->current[0] != 0) {
        sprintf(msg, "busy!\n\r");
        goto _l_error;
    }
    dn = session->argv[1];
    if ((rc = ss_dns_fetch(ctx->service, dn, &addr)) != SS_IO_OK) {
        if (rc != SS_IO_EAGAIN) {
            sprintf(msg, "DNS error %s\n\r", ss_translate_io_err(rc));
            goto _l_error;
        }
    }
    if (rc == SS_IO_OK) {
        ss_addr_format(addrstr, "%a", addr);
        sprintf(msg, "Name: %s\n\rAddress: %s\n\r", dn, addrstr);
        ss_send_via_buffer(&session->response_buffer, msg, strlen(msg));
        session->finished = SS_TRUE;
    } else if (rc == SS_IO_EAGAIN) {
        sprintf(msg, "Name: %s fetching...\n\r", dn);
        ss_send_via_buffer(&session->response_buffer, msg, strlen(msg));
        strcpy(ctx->current, dn);
        time(&now);
        ctx->start_fetch_time = now;
        session->finished = SS_TRUE;
    } else {
        goto _l_error;
    }
    return;
_l_error:
    ss_send_via_buffer(&session->response_buffer, msg, strlen(msg));
    session->finished = SS_TRUE;
}

void ss_testing_comamnd_dns_poll(ss_testing_session_t *session)
{
    ss_testing_command_dns_context_t *ctx;
    char                              msg[1024];
    char                              addrstr[256];
    ss_io_err_t                       rc;
    ss_addr_t                         addr;
    ss_time_t                         now;

    ctx = SS_TESTING_DNS_GET_CONTEXT(session);
    if (ctx->current[0] == 0) return;
    if (!ss_dns_get(ctx->service, ctx->current, &addr)) {
        goto _l_failed;
    }
    goto _l_success;
_l_failed:
    time(&now);
    if (now - ctx->start_fetch_time < FETCH_TIMEOUT) return;
    sprintf(msg, "Name: %s\n\rfetch timeout!\n\r", ctx->current);
    goto _l_end;
_l_success:
    ss_addr_format(addrstr, "%a", addr);
    sprintf(msg, "Name: %s\n\rAddress: %s\n\r", ctx->current, addrstr);
    goto _l_end;
_l_end:
    ss_send_via_buffer(&session->response_buffer, msg, strlen(msg));
    ctx->current[0] = 0;
    session->finished = SS_TRUE;
    return;
}
