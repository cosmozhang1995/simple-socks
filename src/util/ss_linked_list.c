#include "util/ss_linked_list.h"

#include <stdlib.h>

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
    return ss_linked_list_insert(list->tail, data);
}

ss_linked_node_t *ss_linked_list_prepend(ss_linked_list_t *list, void *data)
{
    return ss_linked_list_insert(list->head, data);
}

ss_linked_node_t *ss_linked_list_insert(ss_linked_node_t *before, void *data)
{
    ss_linked_node_t *prev, *node;
    prev = before->prev;
    node = linked_node_create(data);
    if (prev) prev->next = node;
    node->prev = prev;
    node->next = before;
    before->prev = node;
    return node;
}

void ss_linked_list_remove(ss_linked_node_t *node)
{
    ss_linked_node_t *prev, *next;
    prev = node->prev;
    next = node->next;
    if (prev) prev->next = next;
    if (next) next->prev = prev;
    node->prev = 0;
    node->next = 0;
    free(node);
}
