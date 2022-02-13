#ifndef _SS_STRMAP_H_
#define _SS_STRMAP_H_

#include "util/ss_strmap_def.h"

typedef ss_bool_t (*ss_strmap_visitor_t) (const char *key, ss_variable_t *value);

void ss_strmap_initialize(ss_strmap_t *map);

void ss_strmap_uninitialize(ss_strmap_t *map);

ss_bool_t ss_strmap_get(ss_strmap_t *map, const char *key, ss_variable_t *value);

// if old value is replaced, returns true. otherwise returns false
ss_bool_t ss_strmap_put(ss_strmap_t *map, const char *key, ss_variable_t value, ss_variable_t *erased_value);

ss_bool_t ss_strmap_erase(ss_strmap_t *map, const char *key, ss_variable_t *erased_value);

void ss_strmap_foreach(ss_strmap_t *map, ss_strmap_visitor_t function);

void ss_strmap_clear(ss_strmap_t *map, ss_strmap_visitor_t release_function);

#endif
