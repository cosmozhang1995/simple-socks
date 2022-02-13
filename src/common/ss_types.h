#ifndef SOCKS5_COMMON_TYPES_H
#define SOCKS5_COMMON_TYPES_H

typedef unsigned char       ss_uint8_t;
typedef unsigned short      ss_uint16_t;
typedef unsigned int        ss_uint32_t;
typedef unsigned long long  ss_uint64_t;
typedef unsigned long       ss_size_t;
typedef char                ss_int8_t;
typedef short               ss_int16_t;
typedef int                 ss_int32_t;
typedef long long           ss_int64_t;
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
