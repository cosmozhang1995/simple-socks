#include "util/ss_io_helper.h"

const char *ss_translate_io_err(ss_io_err_t err)
{
    switch (err) {
    case SS_IO_OK:
        return "OK";
    case SS_IO_EAGAIN:
        return "EAGAIN";
    case SS_IO_ERROR:
        return "ERROR";
    case SS_IO_EOVERFLOW:
        return "EOVERFLOW";
    default:
        return "<unknown>";
    }
}
