#ifndef _SS_UTIL_SS_PACKET_BUFFER_H_
#define _SS_UTIL_SS_PACKET_BUFFER_H_

#include "util/ss_packet_buffer_def.h"
#include "common/ss_io_error.h"

ss_packet_buffer_t *ss_packet_buffer_create(ss_size_t packet_size, ss_size_t max_packets);
void ss_packet_buffer_destroy(ss_packet_buffer_t *buffer);

void ss_packet_buffer_initialize(ss_packet_buffer_t *buffer, ss_size_t packet_size, ss_size_t max_packets);
void ss_packet_buffer_uninitialize(ss_packet_buffer_t *buffer);

// receive a packet and push it back to the tail of the buffer
ss_io_err_t ss_packet_buffer_recv_back(int fd, ss_packet_buffer_t *buffer, ss_size_t size);

// pop a packet from the front of the buffer and send it
ss_io_err_t ss_packet_buffer_send_front(int fd, ss_packet_buffer_t *buffer);

ss_bool_t ss_packet_buffer_pop_front(ss_packet_buffer_t *buffer, ss_packet_t *packet);

ss_bool_t ss_packet_buffer_push_back(ss_packet_buffer_t *buffer, ss_packet_block_t packet);

ss_bool_t ss_packet_buffer_read_front(void *dest, ss_packet_buffer_t *buffer, ss_size_t *offset, ss_size_t size);
ss_bool_t ss_packet_buffer_write_back(ss_packet_buffer_t *buffer, void *src, ss_size_t *offset, ss_size_t size);


#endif // _SS_UTIL_SS_PACKET_BUFFER_H_
