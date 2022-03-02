#include "common/ss_time.h"

#include <sys/time.h>

ss_timestamp_t ss_current_timestamp()
{
    struct timeval t;
    gettimeofday(&t, 0);
    return (ss_timestamp_t)t.tv_sec * 1000 + t.tv_usec / 1000;
}