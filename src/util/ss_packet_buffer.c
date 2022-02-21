#include "util/ss_packet_buffer.h"

#include <stdlib.h>
#include <memory.h>

#include "common/ss_defs.h"

static ss_inline
ss_packet_buffer_entry_t *get_packet(ss_packet_buffer_t *buffer, size_t idx);

void ss_packet_buffer_create(ss_packet_buffer_t *buffer, ss_size_t capacity, ss_size_t max_packets)
{
    ss_packet_buffer_t *buffer;
    buffer = malloc(sizeof(ss_packet_buffer_t));
    ss_packet_buffer_initialize(buffer, capacity, max_packets);
}
void ss_packet_buffer_destroy(ss_packet_buffer_t *buffer)
{
    ss_packet_buffer_uninitialize(buffer);
    free(buffer);
}

void ss_packet_buffer_initialize(ss_packet_buffer_t *buffer, ss_size_t capacity, ss_size_t max_packets)
{
    memset(buffer, 0, sizeof(ss_packet_buffer_t));
    buffer->dataptr = malloc(capacity);
    buffer->packets = malloc(max_packets * sizeof(ss_packet_buffer_entry_t));
    memset(buffer->packets, 0, max_packets * sizeof(ss_packet_buffer_entry_t));
}

void ss_packet_buffer_uninitialize(ss_packet_buffer_t *buffer)
{
    free(buffer->dataptr);
    free(buffer->packets);
}

ss_io_err_t ss_packet_buffer_recv(int fd, ss_packet_buffer_t *buffer, ss_size_t size)
{
    ss_size_t                  offset;
    ss_packet_buffer_entry_t  *first_packet;
    ss_packet_buffer_entry_t  *last_packet;

    if (size > buffer->capacity)
        return SS_IO_EOVERFLOW;
    if (buffer->packet_count + 1 >= buffer->max_packets)
        return SS_IO_EOVERFLOW;
    // if (buffer->packet_count == 0) {
    //     offset = 0;
    // } else {
    //     last_packet = get_packet(buffer->packets, buffer->packet_count - 1);
    //     offset = last_packet->offset + last_packet->size;
    //     if (offset + size > buffer->capacity) {
    //         first_packet = get_packet(buffer->packets, 0);
    //         if (first_packet->offset < size)
    //             return SS_IO_EOVERFLOW;
    //         offset = 0;
    //     }
    // }
}

ss_io_err_t ss_packet_buffer_send(int fd, ss_packet_buffer_t *buffer, ss_size_t size)
{
}

ss_bool_t ss_packet_buffer_pop_front(int fd, ss_packet_buffer_t *buffer, ss_packet_t *packet);
ss_bool_t ss_packet_buffer_push_back(int fd, ss_packet_buffer_t *buffer, void *data, ss_size_t size);


static ss_inline
ss_packet_buffer_entry_t *get_packet(ss_packet_buffer_t *buffer, size_t idx)
{
    return buffer->packets + (buffer->packet_offset + idx) % (buffer->max_packets);
}


