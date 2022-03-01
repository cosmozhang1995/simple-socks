#ifndef _SS_UTIL_SS_HEAP_DEF_H_
#define _SS_UTIL_SS_HEAP_DEF_H_

#include "common/ss_types.h"

// max heap defination

typedef struct ss_heap_s ss_heap_t;

typedef int (*ss_heap_comparator_t) (ss_variable_t value1, ss_variable_t value2);
typedef ss_bool_t (*ss_heap_visitor_t) (ss_variable_t *value);

struct ss_heap_s {
    ss_variable_t         *values;
    ss_size_t              size;
    ss_size_t              capacity;
    ss_heap_comparator_t   comparator;
};

#endif // _SS_UTIL_SS_HEAP_DEF_H_