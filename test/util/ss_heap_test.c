#include "ss_unittest.h"

#include <stdio.h>

#include "util/ss_heap.h"

static ss_int32_t comparator(ss_variable_t v1, ss_variable_t v2);
static inline ss_variable_t variable(ss_int32_t v);

static void print_heap_data(ss_heap_t *heap);

SS_UNITTEST {
    ss_heap_t      heap;
    ss_variable_t  var;
    ss_size_t      i;

    ss_heap_initialize(&heap, comparator);
    ss_heap_push(&heap, variable(2));
    ss_heap_push(&heap, variable(5));
    ss_heap_push(&heap, variable(4));
    ss_heap_push(&heap, variable(3));
    ss_heap_push(&heap, variable(4));
    ss_heap_push(&heap, variable(2));
    ss_heap_push(&heap, variable(1));
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(1, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(2, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(2, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(3, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(4, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(4, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(5, var.int32);
    ASSERT_FALSE(ss_heap_pop(&heap, &var));

    ss_heap_reserve(&heap, 10);
    i = 0;
    heap.values[i++] = variable(2);
    heap.values[i++] = variable(5);
    heap.values[i++] = variable(4);
    heap.values[i++] = variable(7);
    heap.values[i++] = variable(6);
    heap.values[i++] = variable(3);
    heap.values[i++] = variable(4);
    heap.values[i++] = variable(2);
    heap.values[i++] = variable(8);
    heap.values[i++] = variable(1);
    heap.size = i;
    ss_heap_reconstruct(&heap);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(1, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(2, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(2, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(3, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(4, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(4, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(5, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(6, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(7, var.int32);
    ASSERT_TRUE(ss_heap_pop(&heap, &var));
    ASSERT_EQUAL_INT32(8, var.int32);
    ASSERT_FALSE(ss_heap_pop(&heap, &var));

    ss_heap_uninitialize(&heap);
}

static
ss_int32_t comparator(ss_variable_t v1, ss_variable_t v2)
{
    if (v1.int32 < v2.int32) return 1;
    else if (v1.int32 > v2.int32) return -1;
    else return 0;
}

static inline
ss_variable_t variable(ss_int32_t v)
{
    ss_variable_t var;
    var.int32 = v;
    return var;
}

static void print_heap_data(ss_heap_t *heap)
{
    ss_size_t i;
    for (i = 0; i < heap->size; i++) {
        printf(" %d", heap->values[i].int32);
    }
    printf("\n");
}
