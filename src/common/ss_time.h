#ifndef _SS_COMMON_SS_TIME_H_
#define _SS_COMMON_SS_TIME_H_

#include <time.h>
#include <sys/time.h>

#include "common/ss_defs.h"
#include "common/ss_types.h"

typedef time_t ss_time_t;
typedef ss_uint64_t ss_timestamp_t;

#define SS_TIMESTAMP_MAX SS_UINT64_MAX

static ss_inline ss_timestamp_t ss_current_timestamp()
{
    struct timeval t;
    gettimeofday(&t, 0);
    return (ss_timestamp_t)t.tv_sec * 1000 + t.tv_usec / 1000;
}

#endif // _SS_COMMON_SS_TIME_H_
