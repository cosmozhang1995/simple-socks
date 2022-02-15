#ifndef SOCKS5_BUFFER_H
#define SOCKS5_BUFFER_H

#include <stddef.h>
#include <sys/types.h>

typedef struct {
    void *buffer;
    size_t offset;
    size_t length;
    size_t capacity;
} ss_ring_buffer_t;

ss_ring_buffer_t *ss_ring_buffer_create(size_t capacity);
void ss_ring_buffer_release(ss_ring_buffer_t *buffer);
void ss_ring_buffer_initialize(ss_ring_buffer_t *buffer, size_t capacity);
void ss_ring_buffer_uninitialize(ss_ring_buffer_t *buffer);

// recv data from fd to buffer
ssize_t ss_ring_buffer_recv(int fd, ss_ring_buffer_t *buffer, size_t max_size);

// send data from buffer to fd
ssize_t ss_ring_buffer_send(int fd, ss_ring_buffer_t *buffer, size_t max_size);

// read data from src (ring buffer) to dest (common buffer)
ssize_t ss_ring_buffer_read(void *dest, ss_ring_buffer_t *src, size_t max_size);

// read data from src (ring buffer) to dest (common buffer), but don't affect the source buffer
ssize_t ss_ring_buffer_steal(void *dest, ss_ring_buffer_t *src, size_t offset, size_t max_size);

// read data from src (common buffer) to dest (ring buffer)
ssize_t ss_ring_buffer_write(ss_ring_buffer_t *dest, const void *src, size_t max_size);

// read data from src (ring buffer) to dest (common buffer)
ssize_t ss_ring_buffer_read_fixed(void *dest, ss_ring_buffer_t *src, size_t fixed_size);

// read data from src (ring buffer) to dest (common buffer), but don't affect the source buffer
ssize_t ss_ring_buffer_steal_fixed(void *dest, ss_ring_buffer_t *src, size_t offset, size_t fixed_size);

// read data from src (common buffer) to dest (ring buffer)
ssize_t ss_ring_buffer_write_fixed(ss_ring_buffer_t *dest, const void *src, size_t fixed_size);

size_t ss_ring_buffer_free_size(ss_ring_buffer_t *buffer);

size_t ss_ring_buffer_reset(ss_ring_buffer_t *buffer);

#endif
