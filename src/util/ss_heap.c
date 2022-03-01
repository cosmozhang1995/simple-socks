#include "util/ss_heap.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_CAPACITY 4

ss_heap_t *ss_heap_create(ss_heap_comparator_t comparator)
{
    ss_heap_t *instance = malloc(sizeof(ss_heap_t));
    ss_heap_initialize(instance, comparator);
    return instance;
}

void ss_heap_destroy(ss_heap_t *instance)
{
    ss_heap_uninitialize(instance);
    free(instance);
}

void ss_heap_initialize(ss_heap_t *instance, ss_heap_comparator_t comparator)
{
    memset(instance, 0, sizeof(ss_heap_t));
    instance->values = malloc(DEFAULT_CAPACITY * sizeof(ss_variable_t));
    instance->capacity = DEFAULT_CAPACITY;
    instance->comparator = comparator;
}

void ss_heap_uninitialize(ss_heap_t *instance)
{
    free(instance->values);
}

ss_bool_t ss_heap_push(ss_heap_t *heap, ss_variable_t value)
{
    ss_size_t        new_capacity;
    ss_variable_t   *new_values;
    ss_variable_t   *old_values;
    ss_size_t        i;
    ss_size_t        ip;
    ss_variable_t    tmp;

    if (heap->size == heap->capacity) {
        new_capacity = heap->capacity << 1;
        new_values = malloc(new_capacity * sizeof(ss_variable_t));
        old_values = heap->values;
        memcpy(new_values, old_values, heap->capacity * sizeof(ss_variable_t));
        heap->values = new_values;
        heap->capacity = new_capacity;
        free(old_values);
    }
    heap->values[heap->size++] = value;
    if (heap->size == 1) {
        return SS_TRUE;
    }
    for (i = heap->size - 1; i != 0;) {
        ip = ((i + 1) >> 1) - 1;
        // printf("compare %d with %d result %d\n", heap->values[i].int32, heap->values[ip].int32,
        //     heap->comparator(heap->values[i], heap->values[ip]));
        if (heap->comparator(heap->values[i], heap->values[ip]) <= 0) {
            break;
        }
        tmp = heap->values[i];
        heap->values[i] = heap->values[ip];
        heap->values[ip] = tmp;
        i = ip;
    }
    return SS_TRUE;
}

ss_bool_t ss_heap_pop(ss_heap_t *heap, ss_variable_t *value)
{
    ss_size_t     i;
    ss_size_t     il;
    ss_size_t     ir;
    ss_size_t     in;
    ss_variable_t tmp;

    if (heap->size == 0) {
        return SS_FALSE;
    }
    if (value) {
        *value = heap->values[0];
    }
    if (--heap->size == 0)
        return SS_TRUE;
    heap->values[0] = heap->values[heap->size];
    for (i = 0;;) {
        ir = (i + 1) << 1;
        il = ir - 1;
        if (ir < heap->size) {
            in = heap->comparator(heap->values[il], heap->values[ir]) < 0 ? ir : il;
        } else if (il < heap->size) {
            in = il;
        } else {
            break;
        }
        if (heap->comparator(heap->values[i], heap->values[in]) >= 0) {
            break;
        }
        tmp = heap->values[i];
        heap->values[i] = heap->values[in];
        heap->values[in] = tmp;
        i = in;
    }
    return SS_TRUE;
}

ss_bool_t ss_heap_peak(ss_heap_t *heap, ss_variable_t *value)
{
    if (heap->size == 0) {
        return SS_FALSE;
    }
    if (value) {
        *value = heap->values[0];
    }
    return SS_TRUE;
}

void ss_heap_foreach(ss_heap_t *heap, ss_heap_visitor_t visitor)
{
    for (size_t i = 0; i < heap->size; i++) {
        if (!visitor(heap->values + i)) {
            break;
        }
    }
}

