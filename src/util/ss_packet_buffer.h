#ifndef _SS_UTIL_SS_PACKET_BUFFER_H_
#define _SS_UTIL_SS_PACKET_BUFFER_H_

#include "util/ss_packet_buffer_def.h"
#include "common/ss_io_error.h"

void ss_packet_buffer_create(ss_packet_buffer_t *buffer, ss_size_t capacity, ss_size_t max_packets);
void ss_packet_buffer_destroy(ss_packet_buffer_t *buffer);

void ss_packet_buffer_initialize(ss_packet_buffer_t *buffer, ss_size_t capacity, ss_size_t max_packets);
void ss_packet_buffer_uninitialize(ss_packet_buffer_t *buffer);

ss_io_err_t ss_packet_buffer_recv(int fd, ss_packet_buffer_t *buffer, ss_size_t size);
ss_io_err_t ss_packet_buffer_send(int fd, ss_packet_buffer_t *buffer, ss_size_t size);

ss_bool_t ss_packet_buffer_pop_front(int fd, ss_packet_buffer_t *buffer, ss_packet_t *packet);
ss_bool_t ss_packet_buffer_push_back(int fd, ss_packet_buffer_t *buffer, void *data, ss_size_t size);

#endif // _SS_UTIL_SS_PACKET_BUFFER_H_
