#ifndef SOCKS5_STRING_UTILS_H
#define SOCKS5_STRING_UTILS_H

#include "common/ss_types.h"

void ss_string_trim(char *);
ss_uint32_t ss_string_hash(const char *);
char *ss_string_clone(const char *);
ss_bool_t ss_string_empty(const char *);

#endif
