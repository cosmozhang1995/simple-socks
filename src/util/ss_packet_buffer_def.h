#ifndef _SS_UTIL_SS_PACKET_BUFFER_DEF_H_
#define _SS_UTIL_SS_PACKET_BUFFER_DEF_H_

#include "common/ss_types.h"

typedef struct ss_packet_buffer_s ss_packet_buffer_t;
typedef struct ss_packet_block_s ss_packet_block_t;
typedef struct ss_packet_s ss_packet_t;

struct ss_packet_buffer_s {
    void           *dataptr;
    ss_size_t       capacity;
    ss_size_t       max_packets;
    ss_size_t       packet_size;
    ss_size_t       packet_offset;
    ss_size_t       packet_count;
};

struct ss_packet_block_s {
    ss_size_t       size;
};

struct ss_packet_s {
    ss_size_t       size;
    void           *dataptr;
};

#endif // _SS_UTIL_SS_PACKET_BUFFER_DEF_H_
