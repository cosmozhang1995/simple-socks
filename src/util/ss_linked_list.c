#include "util/ss_linked_list.h"

#include <stdlib.h>

#include "common/ss_defs.h"
#include "common/ss_types.h"

static ss_inline void linkde_node_detach(ss_linked_node_t *node);
static ss_inline void linked_node_insert_before(ss_linked_node_t *node, ss_linked_node_t *before);

static ss_linked_node_t *linked_node_create(void *data)
{
    ss_linked_node_t *node = malloc(sizeof(ss_linked_node_t));
    node->data = data;
    node->prev = 0;
    node->next = 0;
    return node;
}
static void linked_node_release(ss_linked_node_t *node)
{
    free(node);
}

ss_linked_list_t *ss_linked_list_create()
{
    ss_linked_list_t *list = malloc(sizeof(ss_linked_list_t));
    ss_linked_list_initialize(list);
    return list;
}

void ss_linked_list_initialize(ss_linked_list_t *list)
{
    list->head = linked_node_create(0);
    list->tail = linked_node_create(0);
    list->head->next = list->tail;
    list->tail->prev = list->head;
}

void ss_linked_list_uninitialize(ss_linked_list_t *list,
    ss_linked_list_release_data_function_t release_data_function)
{
    ss_linked_node_t *cnode, *nnode;
    cnode = list->head;
    while (cnode) {
        if (cnode->data) {
            release_data_function(cnode->data);
        }
        nnode = cnode->next;
        free(cnode);
        cnode = nnode;
    }
}

void ss_linked_list_release(ss_linked_list_t *list, ss_linked_list_release_data_function_t release_data_function)
{
    ss_linked_list_uninitialize(list, release_data_function);
    free(list);
}

ss_linked_node_t *ss_linked_list_append(ss_linked_list_t *list, void *data)
{
    ss_linked_node_t *node;
    node = linked_node_create(data);
    linked_node_insert_before(node, list->tail);
    return node;
}

ss_linked_node_t *ss_linked_list_prepend(ss_linked_list_t *list, void *data)
{
    ss_linked_node_t *node;
    node = linked_node_create(data);
    linked_node_insert_before(node, list->head->next);
    return node;
}

ss_linked_node_t *ss_linked_list_insert(ss_linked_node_t *before, void *data)
{
    ss_linked_node_t *node;
    node = linked_node_create(data);
    linked_node_insert_before(node, before);
    return node;
}

ss_linked_node_t *ss_linked_list_append_node(ss_linked_list_t *list, ss_linked_node_t *node)
{
    linked_node_insert_before(node, list->tail);
    return node;
}

ss_linked_node_t *ss_linked_list_prepend_node(ss_linked_list_t *list, ss_linked_node_t *node)
{
    linked_node_insert_before(node, list->head);
    return node;
}

ss_linked_node_t *ss_linked_list_insert_node(ss_linked_node_t *before, ss_linked_node_t *node)
{
    linked_node_insert_before(node, before);
    return node;
}

ss_linked_node_t *ss_linked_list_pop_node_front(ss_linked_list_t *list)
{
    ss_linked_node_t *node;
    node = list->head->next;
    if (node == SS_NULL || node == list->tail) {
        return SS_NULL;
    }
    linkde_node_detach(node);
    return node;
}

void ss_linked_list_remove(ss_linked_node_t *node)
{
    linkde_node_detach(node);
    free(node);
}

void ss_linked_list_detach(ss_linked_node_t *node)
{
    linkde_node_detach(node);
}

ss_size_t ss_linked_list_length(ss_linked_list_t *list)
{
    ss_linked_node_t *node;
    ss_size_t         length;

    length = 0;
    for (
        node = list->head->next;
        node != SS_NULL && node != list->tail;
        node = node->next
    )
        length++;
    return length;
}






static ss_inline
void linkde_node_detach(ss_linked_node_t *node)
{
    ss_linked_node_t *prev, *next;
    prev = node->prev;
    next = node->next;
    if (prev) prev->next = next;
    if (next) next->prev = prev;
    node->prev = 0;
    node->next = 0;
}

static ss_inline
void linked_node_insert_before(ss_linked_node_t *node, ss_linked_node_t *before)
{
    ss_linked_node_t *prev;
    prev = before->prev;
    if (prev) prev->next = node;
    node->prev = prev;
    node->next = before;
    before->prev = node;
}
