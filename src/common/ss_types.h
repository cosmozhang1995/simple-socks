#ifndef SOCKS5_COMMON_TYPES_H
#define SOCKS5_COMMON_TYPES_H

typedef unsigned char       ss_uint8_t;
#define SS_UINT8_MAX        0xFF

typedef unsigned short      ss_uint16_t;
#define SS_UINT16_MAX       0xFFFF

typedef unsigned int        ss_uint32_t;
#define SS_UINT32_MAX       0xFFFFFFFF

typedef unsigned long long  ss_uint64_t;
#define SS_UINT64_MAX       0xFFFFFFFFFFFFFFFF

typedef unsigned long       ss_size_t;
#define SS_SIZE_MAX         0x0
#define SS_SIZE_MAIN        0x0

typedef char                ss_int8_t;
#define SS_INT8_MAX         0x0
#define SS_INT8_MAIN        0x0

typedef short               ss_int16_t;
#define SS_INT16_MAX        0x0
#define SS_INT16_MIN        0x0

typedef int                 ss_int32_t;
#define SS_INT32_MAX        0x0
#define SS_INT32_MIN        0x0

typedef long long           ss_int64_t;
#define SS_INT64_MAX        0x0
#define SS_INT64_MIN        0x0

typedef long                ss_ssize_t;

typedef unsigned char       ss_bool_t;
#define SS_TRUE             1
#define SS_FALSE            0

#define SS_NULL             0

typedef union {
    void            *ptr;
    ss_uint8_t       uint8;
    ss_uint16_t      uint16;
    ss_uint32_t      uint32;
    ss_uint64_t      uint64;
    ss_size_t        size;
    ss_int8_t        int8;
    ss_int16_t       int16;
    ss_int32_t       int32;
    ss_int64_t       int64;
    ss_ssize_t       ssize;
} ss_variable_t;

#endif
