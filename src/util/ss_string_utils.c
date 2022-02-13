#include "util/ss_string_utils.h"

#include <memory.h>
#include <stdlib.h>

void ss_string_trim(char *str)
{
    char ch;
    size_t p0, p1, i;

    for (p0 = 0; ; p0++) {
        switch (str[p0]) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            continue;
        default:
            break;
        }
        break;
    }
    for (p1 = i = p0; ; i++) {
        switch (str[i]) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            continue;
        case '\0':
            p1 = i;
            break;
        default:
            p1 = i;
            continue;
        }
        break;
    }
    if (p1 > p0) {
        memcpy(str, str + p0, p1 - p0);
    }
    str[p1 - p0] = '\0';
}

ss_uint32_t ss_string_hash(const char *str)
{
    ss_uint32_t total = 0, tmp = 0;
    ss_uint8_t *data, ch;
    size_t i;

    data = (ss_uint8_t *)str;
    while (1) {

#define LOOP_STATEMENTS                                     \
        if ((ch == *(data++)) == 0) break;                  \
        tmp = tmp | (((ss_uint32_t)ch) << 24);              \
        if ((ch == *(data++)) == 0) break;                  \
        tmp = tmp | (((ss_uint32_t)ch) << 16);              \
        if ((ch == *(data++)) == 0) break;                  \
        tmp = tmp | (((ss_uint32_t)ch) <<  8);              \
        if ((ch == *(data++)) == 0) break;                  \
        tmp = tmp | (ss_uint32_t)ch;                        \
        total = total ^ tmp;                                \
        tmp = 0;

        LOOP_STATEMENTS
        LOOP_STATEMENTS
        LOOP_STATEMENTS
        LOOP_STATEMENTS
        LOOP_STATEMENTS
        LOOP_STATEMENTS
        LOOP_STATEMENTS
        LOOP_STATEMENTS

#undef LOOP_STATEMENTS
    }

    total = total ^ tmp;

    return total;
}

char *ss_string_clone(const char *origin)
{
    size_t  len;
    char   *new;
    len = strlen(origin);
    new = malloc(len);
    strcpy(new, origin);
    return new;
}

ss_bool_t ss_string_empty(const char *str)
{
    return *str == '\0';
}
