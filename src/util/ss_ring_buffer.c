#include "util/ss_ring_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>

ss_ring_buffer_t *ss_ring_buffer_create(size_t capacity)
{
    ss_ring_buffer_t *buffer = malloc(sizeof(ss_ring_buffer_t));
    ss_ring_buffer_initialize(buffer, capacity);
    return buffer;
}

void ss_ring_buffer_release(ss_ring_buffer_t *buffer)
{
    ss_ring_buffer_uninitialize(buffer);
    free(buffer);
}

void ss_ring_buffer_initialize(ss_ring_buffer_t *buffer, size_t capacity)
{
    buffer->buffer = malloc(capacity);
    buffer->capacity = capacity;
    ss_ring_buffer_reset(buffer);
}

void ss_ring_buffer_uninitialize(ss_ring_buffer_t *buffer)
{
    if (buffer->buffer) free(buffer->buffer);
    buffer->buffer = 0;
    buffer->capacity = 0;
    ss_ring_buffer_reset(buffer);
}

static int handle_recv_result(ssize_t result, size_t desired_recv_bytes, ss_ring_buffer_t *buffer)
{
    int recv_errno;
    if (result == 0) {
        return 0;
    }
    if (result < 0) {
        switch (recv_errno = errno)
        {
        case EAGAIN:
            // no more data to read
            break;
        default:
            printf("recv got error %d\n", recv_errno);
            break;
        }
        return 0;
    }
    buffer->length += result;
    if (result < desired_recv_bytes) {
        return 0;
    } else {
        return 1;
    }
}

ssize_t ss_ring_buffer_recv(int fd, ss_ring_buffer_t *buffer, size_t max_size)
{
    size_t total_nbytes_to_read, nbytes_to_read;
    ssize_t recv_ret;
    size_t init_lenght, total_read;
    size_t offset;

    init_lenght = buffer->length;
    
    total_nbytes_to_read = max_size;
    if (total_nbytes_to_read == 0)
        goto _l_end;
    offset = buffer->offset + buffer->length;
    if (offset < buffer->capacity) {
        nbytes_to_read = buffer->capacity - offset;
        if (nbytes_to_read > total_nbytes_to_read) nbytes_to_read = total_nbytes_to_read;
        recv_ret = recv(fd, buffer->buffer + offset, nbytes_to_read, 0);
        if (!handle_recv_result(recv_ret, nbytes_to_read, buffer))
            goto _l_end;
    }
    
    total_nbytes_to_read -= (size_t)recv_ret;
    if (total_nbytes_to_read == 0)
        goto _l_end;
    offset = (buffer->offset + buffer->length) % buffer->capacity;
    if (offset < buffer->offset) {
        nbytes_to_read = buffer->offset - offset;
        if (nbytes_to_read > total_nbytes_to_read) nbytes_to_read = total_nbytes_to_read;
        recv_ret = recv(fd, buffer->buffer + offset, nbytes_to_read, 0);
        if (!handle_recv_result(recv_ret, nbytes_to_read, buffer))
            goto _l_end;
    }

_l_end:
    total_read = buffer->length - init_lenght;
    return (ssize_t)total_read;
}

#define MAX(x, y) ((x) < (y) ? y : x)
#define MIN(x, y) ((x) < (y) ? x : y)
#define CHUNK_SIZE 512
ssize_t ss_ring_buffer_send(int fd, ss_ring_buffer_t *buffer, size_t max_size)
{
    size_t total_nbytes_to_write, nbytes_to_write;
    ssize_t nbytes_written;
    size_t init_length, total_written;

    init_length = buffer->length;
    total_nbytes_to_write = max_size;
    while (total_nbytes_to_write > 0) {
        nbytes_to_write = MIN(buffer->length, CHUNK_SIZE);
        if (buffer->offset + nbytes_to_write > buffer->capacity)
            nbytes_to_write = buffer->capacity - buffer->offset;
        if (nbytes_to_write > total_nbytes_to_write)
            nbytes_to_write = total_nbytes_to_write;
        nbytes_written = send(fd, (void*)(buffer->buffer + buffer->offset), nbytes_to_write, MSG_NOSIGNAL);
        if (nbytes_written <= 0)
            break;
        buffer->offset = (buffer->offset + (size_t)nbytes_written) % buffer->capacity;
        buffer->length -= nbytes_written;
        total_nbytes_to_write -= (size_t)nbytes_written;
    }

    total_written = buffer->length - init_length;
    return (ssize_t)total_written;
}

ssize_t ss_ring_buffer_read(void *dest, ss_ring_buffer_t *src, size_t max_size)
{
    size_t nbytes_to_read;
    nbytes_to_read = MIN(src->length, max_size);
    return ss_ring_buffer_read_fixed(dest, src, nbytes_to_read);
}

ssize_t ss_ring_buffer_steal(void *dest, ss_ring_buffer_t *src, size_t offset, size_t max_size)
{
    size_t nbytes_to_read;
    if (src->length < offset) return 0;
    nbytes_to_read = MIN(src->length - offset, max_size);
    return ss_ring_buffer_steal_fixed(dest, src, offset, nbytes_to_read);
}

ssize_t ss_ring_buffer_write(ss_ring_buffer_t *dest, void *src, size_t max_size)
{
    size_t nbytes_to_write;
    nbytes_to_write = MIN(dest->capacity - dest->length, max_size);
    return ss_ring_buffer_write_fixed(dest, src, nbytes_to_write);
}

ssize_t ss_ring_buffer_read_fixed(void *dest, ss_ring_buffer_t *src, size_t fixed_size)
{
    size_t nbytes;

    if (src->length < fixed_size) return -1;
    if (!dest) {
        src->offset = (src->offset + fixed_size) % src->capacity;
        src->length -= fixed_size;
        goto _l_end;
    }
    nbytes = fixed_size;
    if (src->offset + nbytes > src->capacity)
        nbytes = src->capacity - src->offset;
    if (nbytes == 0) goto _l_end;
    memcpy(dest, src->buffer + src->offset, nbytes);
    src->offset = (src->offset + nbytes) % src->capacity;
    src->length -= nbytes;
    dest += nbytes;
    nbytes = fixed_size - nbytes;
    if (nbytes == 0) goto _l_end;
    memcpy(dest, src->buffer + src->offset, nbytes);
    src->offset = (src->offset + nbytes) % src->capacity;
    src->length -= nbytes;

_l_end:
    return (ssize_t)fixed_size;
}

ssize_t ss_ring_buffer_steal_fixed(void *dest, ss_ring_buffer_t *src, size_t offset, size_t fixed_size)
{
    size_t nbytes;
    size_t length, capacity;

    if (src->length < offset + fixed_size) return -1;
    if (!dest) goto _l_end;
    nbytes = fixed_size;
    capacity = src->capacity;
    length = src->length - offset;
    offset = (src->offset + offset) % capacity;
    nbytes = fixed_size;
    if (offset + nbytes > capacity)
        nbytes = capacity - offset;
    if (nbytes == 0) goto _l_end;
    memcpy(dest, src->buffer + offset, nbytes);
    offset = (offset + nbytes) % capacity;
    length -= nbytes;
    dest += nbytes;
    nbytes = fixed_size - nbytes;
    if (nbytes == 0) goto _l_end;
    memcpy(dest, src->buffer + offset, nbytes);
    offset = (offset + nbytes) % capacity;
    length -= nbytes;

_l_end:
    return (ssize_t)fixed_size;
}

ssize_t ss_ring_buffer_write_fixed(ss_ring_buffer_t *dest, void *src, size_t fixed_size)
{
    size_t nbytes;
    size_t offset;

    if (dest->length + fixed_size > dest->capacity) return -1;
    nbytes = fixed_size;
    offset = (dest->offset + dest->length) % dest->capacity;
    if (offset + nbytes > dest->capacity)
        nbytes = dest->capacity - offset;
    if (nbytes == 0) goto _l_end;
    memcpy(dest->buffer + offset, src, nbytes);
    dest->length += nbytes;
    src += nbytes;
    nbytes = fixed_size - nbytes;
    if (nbytes == 0) goto _l_end;
    offset = 0;
    memcpy(dest->buffer + offset, src, nbytes);

_l_end:
    return (ssize_t)fixed_size;
}

size_t ss_ring_buffer_free_size(ss_ring_buffer_t *buffer)
{
    return buffer->capacity - buffer->length;
}

size_t ss_ring_buffer_reset(ss_ring_buffer_t *buffer)
{
    buffer->offset = 0;
    buffer->length = 0;
}
