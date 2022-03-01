#ifndef _SS_UTIL_SS_HEAP_H_
#define _SS_UTIL_SS_HEAP_H_

#include "util/ss_heap_def.h"

ss_heap_t *ss_heap_create(ss_heap_comparator_t);
void ss_heap_destroy(ss_heap_t *);
void ss_heap_initialize(ss_heap_t *, ss_heap_comparator_t);
void ss_heap_uninitialize(ss_heap_t *);

ss_bool_t ss_heap_push(ss_heap_t *heap, ss_variable_t value);
ss_bool_t ss_heap_pop(ss_heap_t *heap, ss_variable_t *value);
ss_bool_t ss_heap_peak(ss_heap_t *heap, ss_variable_t *value);
void ss_heap_foreach(ss_heap_t *, ss_heap_visitor_t);

#endif // _SS_UTIL_SS_HEAP_H_