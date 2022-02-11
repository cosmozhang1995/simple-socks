#include "util/linked_list.h"

#include <stdlib.h>

static linked_node_t *linked_node_create(void *data)
{
    linked_node_t *node = malloc(sizeof(linked_node_t));
    node->data = data;
    node->prev = 0;
    node->next = 0;
    return node;
}
static void linked_node_release(linked_node_t *node)
{
    free(node);
}

linked_list_t *linked_list_create()
{
    linked_list_t *list = malloc(sizeof(linked_list_t));
    list->head = linked_node_create(0);
    list->tail = linked_node_create(0);
    list->head->next = list->tail;
    list->tail->prev = list->head;
    return list;
}

void linked_list_release(linked_list_t *list, release_data_function_t release_data_function)
{
    linked_node_t *cnode, *nnode;
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

linked_node_t *linked_list_append(linked_list_t *list, void *data)
{
    return linked_list_insert(list->tail, data);
}

linked_node_t *linked_list_prepend(linked_list_t *list, void *data)
{
    return linked_list_insert(list->head, data);
}

linked_node_t *linked_list_insert(linked_node_t *before, void *data)
{
    linked_node_t *prev, *node;
    prev = before->prev;
    node = linked_node_create(data);
    if (prev) prev->next = node;
    node->prev = prev;
    node->next = before;
    before->prev = node;
    return node;
}

void linked_list_remove(linked_node_t *node)
{
    linked_node_t *prev, *next;
    prev = node->prev;
    next = node->next;
    if (prev) prev->next = next;
    if (next) next->prev = prev;
    node->prev = 0;
    node->next = 0;
    free(node);
}
