#ifndef _SS_UTIL_SS_PACKET_BUFFER_DEF_H_
#define _SS_UTIL_SS_PACKET_BUFFER_DEF_H_

#include "common/ss_types.h"

typedef struct ss_packet_buffer_s ss_packet_buffer_t;
typedef struct ss_packet_buffer_entry_s ss_packet_buffer_entry_t;

typedef struct ss_packet_s ss_packet_t;

struct ss_packet_buffer_s {
    void           *dataptr;
    ss_packet_t    *packets;
    ss_size_t       capacity;
    ss_size_t       max_packets;
    ss_size_t       packet_offset;
    ss_size_t       packet_count;
    ss_size_t       data_offset;
    ss_size_t       data_size;
};

struct ss_packet_buffer_entry_s {
    ss_size_t       offset;
    ss_size_t       size;
};

struct ss_packet_s {
    void           *dataptr;
    ss_size_t       size;
};

#endif // _SS_UTIL_SS_PACKET_BUFFER_DEF_H_
