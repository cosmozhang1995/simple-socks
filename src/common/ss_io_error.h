#ifndef _SS_COMMON_SS_IO_ERROR_H_
#define _SS_COMMON_SS_IO_ERROR_H_

#include "common/ss_types.h"

typedef ss_int8_t ss_io_err_t;

#define SS_IO_OK            0
#define SS_IO_EAGAIN        1
#define SS_IO_ERROR        -1
#define SS_IO_EOVERFLOW    -2
#define SS_IO_EEMPTY       -3

#endif // _SS_COMMON_SS_IO_ERROR_H_
