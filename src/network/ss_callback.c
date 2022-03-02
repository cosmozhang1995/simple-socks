#include "network/ss_callback.h"
#include "network/ss_callback_sys.h"

#include <stdlib.h>
#include <memory.h>

#include "util/ss_heap.h"

#define MAX_QUEUE_SIZE 0x4000

ss_callback_context_t *ss_callback_create(ss_callback_handler_t handler, ss_time_t timeout);
void ss_callback_destroy(ss_callback_context_t *callback);
void ss_callback_initialize(ss_callback_context_t *callback, ss_callback_handler_t handler, ss_time_t timeout);
void ss_callback_uninitialize(ss_callback_context_t *callback);

static ss_heap_t callback_queue;

static ss_int32_t ss_callback_queue_comparartor(ss_variable_t v1, ss_variable_t v2);
static ss_bool_t ss_callback_queue_stop_handler(ss_variable_t *v);

static void ss_callback_clean();

void ss_callback_prepare()
{
    ss_heap_initialize(&callback_queue, ss_callback_queue_comparartor);
}

void ss_callback_stop()
{
    ss_heap_foreach(&callback_queue, ss_callback_queue_stop_handler);
    ss_heap_uninitialize(&callback_queue);
}

ss_time_t ss_callback_poll()
{
    ss_size_t                   i;
    ss_variable_t               var;
    ss_callback_context_t      *cb;
    ss_timestamp_t              now;
    ss_time_t                   next_timeout;

    next_timeout = SS_TIMESTAMP_MAX;
    if (callback_queue.size == 0) {
        return next_timeout;
    }
    now = ss_current_timestamp();
    while (ss_heap_peak(&callback_queue, &var)) {
        cb = (ss_callback_context_t *)var.ptr;
        if (cb->status != SS_CALLBACK_INIT) {
            ss_heap_pop(&callback_queue, SS_NULL);
            ss_callback_destroy(cb);
        } else if (cb->expire <= now) {
            ss_heap_pop(&callback_queue, SS_NULL);
            if (cb->handler) {
                cb->handler(cb);
            }
            ss_callback_destroy(cb);
        } else {
            next_timeout = (ss_time_t)(cb->expire - now);
            break;
        }
    }
    return next_timeout;
}

ss_callback_context_t *ss_callback_register(ss_callback_handler_t handler, ss_time_t timeout)
{
    ss_callback_context_t      *cb;
    if (callback_queue.size != 0 && callback_queue.size % MAX_QUEUE_SIZE == 0) {
        ss_callback_clean();
    }
    cb = ss_callback_create(handler, timeout);
}

ss_callback_context_t *ss_callback_create(ss_callback_handler_t handler, ss_time_t timeout)
{
    ss_callback_context_t      *cb;
    cb = malloc(sizeof(ss_callback_context_t));
    ss_callback_initialize(cb, handler, timeout);
    return cb;
}

void ss_callback_destroy(ss_callback_context_t *cb)
{
    ss_callback_uninitialize(cb);
    free(cb);
}

void ss_callback_initialize(ss_callback_context_t *cb, ss_callback_handler_t handler, ss_time_t timeout)
{
    memset(cb, 0, sizeof(ss_callback_context_t));
    cb->handler = handler;
    cb->expire = ss_current_timestamp() + timeout;
    cb->status = SS_CALLBACK_INIT;
}

void ss_callback_uninitialize(ss_callback_context_t *cb)
{
    cb->status = SS_CALLBACK_TIMEOUT;
}


static ss_int32_t ss_callback_queue_comparartor(ss_variable_t v1, ss_variable_t v2)
{
    ss_callback_context_t      *c1;
    ss_callback_context_t      *c2;
    c1 = (ss_callback_context_t *)v1.ptr;
    c2 = (ss_callback_context_t *)v2.ptr;
    if (c1->expire < c2->expire) return 1;
    else if (c1->expire > c2->expire) return -1;
    else return 0;
}

static ss_bool_t ss_callback_queue_stop_handler(ss_variable_t *v)
{
    ss_callback_destroy((ss_callback_context_t *)v->ptr);
}

static void ss_callback_clean()
{
    size_t                      i;
    size_t                      j;
    ss_callback_context_t      *cb;
    ss_timestamp_t              now;
    ss_variable_t               var;

    now = ss_current_timestamp();
    for (i = 0, j = 0; i < callback_queue.size; i++) {
        var = callback_queue.values[i];
        cb = var.ptr;
        if (cb->status != SS_CALLBACK_INIT) {
            ss_heap_pop(&callback_queue, SS_NULL);
            ss_callback_destroy(cb);
        } else if (cb->expire <= now) {
            ss_heap_pop(&callback_queue, SS_NULL);
            if (cb->handler) {
                cb->handler(cb);
            }
            ss_callback_destroy(cb);
        } else {
            callback_queue.values[j++] = var;
        }
    }
    callback_queue.size = j;
}

