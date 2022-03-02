#ifndef _SS_COMMON_SS_TIME_H_
#define _SS_COMMON_SS_TIME_H_

#include <time.h>

#include "common/ss_types.h"

typedef time_t ss_time_t;
typedef ss_uint64_t ss_timestamp_t;

#define SS_TIMESTAMP_MAX SS_UINT64_MAX

ss_timestamp_t ss_current_timestamp();

#endif // _SS_COMMON_SS_TIME_H_
