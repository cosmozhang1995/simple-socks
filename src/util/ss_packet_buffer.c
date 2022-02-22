#include "util/ss_packet_buffer.h"

#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "common/ss_defs.h"

#define BLOCK_SIZE(packet_size) (sizeof(ss_packet_block_t) + (packet_size))
#define REAL_BLOCK_INDEX(buffer, idx) ((buffer->packet_offset + (idx)) % buffer->max_packets)

static ss_inline
ss_packet_block_t *get_block(ss_packet_buffer_t *buffer, size_t idx);

static ss_inline
void *get_block_data(ss_packet_block_t *blockptr);

ss_packet_buffer_t *ss_packet_buffer_create(ss_size_t packet_size, ss_size_t max_packets)
{
    ss_packet_buffer_t *buffer;
    buffer = malloc(sizeof(ss_packet_buffer_t));
    ss_packet_buffer_initialize(buffer, packet_size, max_packets);
    return buffer;
}
void ss_packet_buffer_destroy(ss_packet_buffer_t *buffer)
{
    ss_packet_buffer_uninitialize(buffer);
    free(buffer);
}

void ss_packet_buffer_initialize(ss_packet_buffer_t *buffer, ss_size_t packet_size, ss_size_t max_packets)
{
    memset(buffer, 0, sizeof(ss_packet_buffer_t));
    buffer->dataptr = malloc(BLOCK_SIZE(packet_size) * max_packets);
    buffer->packet_count = 0;
    buffer->packet_offset = 0;
    buffer->packet_size = packet_size;
    buffer->max_packets = max_packets;
}

void ss_packet_buffer_uninitialize(ss_packet_buffer_t *buffer)
{
    free(buffer->dataptr);
}

ss_io_err_t ss_packet_buffer_recv_back(int fd, ss_packet_buffer_t *buffer, ss_size_t size)
{
    void                      *dataptr;
    ss_packet_block_t         *block;
    ssize_t                    rc;

    if (size > buffer->packet_size)
        return SS_IO_EOVERFLOW;
    if (size == 0)
        return SS_IO_EEMPTY;
    if (buffer->packet_count + 1 >= buffer->max_packets)
        return SS_IO_EOVERFLOW;
    block = get_block(buffer, buffer->packet_count);
    dataptr = get_block_data(block);
    if ((rc = recv(fd, dataptr, size, 0)) < 0) {
        return SS_IO_ERROR;
    }
    if (rc == 0) {
        return SS_IO_EAGAIN;
    }
    block->size = rc;
    buffer->packet_count++;
    return SS_IO_OK;
}

ss_io_err_t ss_packet_buffer_send_front(int fd, ss_packet_buffer_t *buffer)
{
    void                      *dataptr;
    ss_packet_block_t  *block;
    ssize_t                    rc;

    if (buffer->packet_count == 0)
        return SS_IO_EOVERFLOW;
    block = get_block(buffer, 0);
    if (block->size == 0) {
        buffer->packet_offset = (buffer->packet_offset + 1) % buffer->max_packets;
        buffer->packet_count--;
        return SS_IO_EEMPTY;
    }
    dataptr = get_block_data(block);
    if ((rc = send(fd, dataptr, block->size, 0)) != block->size) {
        return SS_IO_ERROR;
    }
    block->size = 0;
    buffer->packet_offset = (buffer->packet_offset + 1) % buffer->max_packets;
    buffer->packet_count--;
    return SS_IO_OK;
}

ss_bool_t ss_packet_buffer_pop_front(ss_packet_buffer_t *buffer, ss_packet_t *packet)
{
    ss_packet_block_t         *block;
    void                      *dataptr;

    if (buffer->packet_count == 0)
        return SS_FALSE;
    block = get_block(buffer, 0);
    dataptr = get_block_data(block);
    if (packet) {
        packet->dataptr = dataptr;
        packet->size = block->size;
    }
    buffer->packet_offset = (buffer->packet_offset + 1) % buffer->max_packets;
    buffer->packet_count--;
    return SS_TRUE;
}

ss_bool_t ss_packet_buffer_push_back(ss_packet_buffer_t *buffer, ss_packet_block_t packet)
{
    ss_packet_block_t  *block;

    block = get_block(buffer, buffer->packet_count);
    memcpy(block, &packet, sizeof(ss_packet_block_t));
    buffer->packet_count++;
    return SS_TRUE;
}

ss_bool_t ss_packet_buffer_read_front(void *dest, ss_packet_buffer_t *buffer, ss_size_t *offset_ptr, ss_size_t size)
{
    void                      *dataptr;
    ss_packet_block_t         *block;
    ss_size_t                  offset;

    if (buffer->packet_count == 0)
        return SS_FALSE;
    offset = *offset_ptr;
    block = get_block(buffer, 0);
    if (offset + size > block->size)
        return SS_FALSE;
    dataptr = get_block_data(block);
    if (dest) {
        memcpy(dest, dataptr + offset, size);
    }
    *offset_ptr = offset + size;
    return SS_TRUE;
}

ss_bool_t ss_packet_buffer_write_back(ss_packet_buffer_t *buffer, void *src, ss_size_t *offset_ptr, ss_size_t size)
{
    void                      *dataptr;
    ss_packet_block_t         *block;
    ss_size_t                  offset;

    offset = offset_ptr ? *offset_ptr : 0;
    if (offset + size > buffer->packet_size)
        return SS_FALSE;
    if (buffer->packet_count + 1 >= buffer->max_packets)
        return SS_FALSE;
    block = get_block(buffer, buffer->packet_count);
    dataptr = get_block_data(block);
    memcpy(dataptr + offset, src, size);
    if (offset_ptr) *offset_ptr = offset + size;
    return SS_TRUE;
}


static ss_inline
ss_packet_block_t *get_block(ss_packet_buffer_t *buffer, size_t idx)
{
    return (ss_packet_block_t *)
        (buffer->dataptr + REAL_BLOCK_INDEX(buffer, idx) * BLOCK_SIZE(buffer->packet_size));
}

static ss_inline
void *get_block_data(ss_packet_block_t *blockptr)
{
    return (void *)blockptr + sizeof(ss_packet_block_t);
}


