#ifndef SOCKS5_LINKED_LIST_H
#define SOCKS5_LINKED_LIST_H

typedef struct linked_node_s {
    void *data;
    struct linked_node_s *prev;
    struct linked_node_s *next;
} ss_linked_node_t;

typedef struct linked_list_s {
    struct linked_node_s *head;
    struct linked_node_s *tail;
} ss_linked_list_t;

typedef void (*ss_linked_list_release_data_function_t)(void*);

ss_linked_list_t *ss_linked_list_create();
void ss_linked_list_release(ss_linked_list_t *list,
    ss_linked_list_release_data_function_t release_data_function);
ss_linked_node_t *ss_linked_list_append(ss_linked_list_t *list, void *data);
ss_linked_node_t *ss_linked_list_prepend(ss_linked_list_t *list, void *data);
ss_linked_node_t *ss_linked_list_insert(ss_linked_node_t *before, void *data);
void ss_linked_list_remove(ss_linked_node_t *node);

#endif
