#ifndef _SS_CRC32_H_
#define _SS_CRC32_H_

#include <stddef.h>

#include "common/ss_types.h"
#include "common/ss_defs.h"

extern ss_uint32_t  *ss_crc32_table_short;
extern ss_uint32_t   ss_crc32_table256[];


static ss_inline ss_uint32_t
ss_crc32_short(ss_uint8_t *p, size_t len)
{
    ss_uint8_t    c;
    ss_uint32_t   crc;

    crc = 0xffffffff;

    while (len--) {
        c = *p++;
        crc = ss_crc32_table_short[(crc ^ (c & 0xf)) & 0xf] ^ (crc >> 4);
        crc = ss_crc32_table_short[(crc ^ (c >> 4)) & 0xf] ^ (crc >> 4);
    }

    return crc ^ 0xffffffff;
}


static ss_inline ss_uint32_t
ss_crc32_long(ss_uint8_t *p, size_t len)
{
    ss_uint32_t  crc;

    crc = 0xffffffff;

    while (len--) {
        crc = ss_crc32_table256[(crc ^ *p++) & 0xff] ^ (crc >> 8);
    }

    return crc ^ 0xffffffff;
}


#define ss_crc32_init(crc)                                                   \
    crc = 0xffffffff


static ss_inline void
ss_crc32_update(ss_uint32_t *crc, ss_uint8_t *p, size_t len)
{
    ss_uint32_t  c;

    c = *crc;

    while (len--) {
        c = ss_crc32_table256[(c ^ *p++) & 0xff] ^ (c >> 8);
    }

    *crc = c;
}


#define ss_crc32_final(crc)                                                  \
    crc ^= 0xffffffff


// ss_int32_t ss_crc32_table_init(void);

#endif
