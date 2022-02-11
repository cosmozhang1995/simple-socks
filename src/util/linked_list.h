typedef struct linked_node_s {
    void *data;
    struct linked_node_s *prev;
    struct linked_node_s *next;
} linked_node_t;

typedef struct linked_list_s {
    struct linked_node_s *head;
    struct linked_node_s *tail;
} linked_list_t;

typedef void (*release_data_function_t)(void*);

linked_list_t *linked_list_create();
void linked_list_release(linked_list_t *list, release_data_function_t release_data_function);
linked_node_t *linked_list_append(linked_list_t *list, void *data);
linked_node_t *linked_list_prepend(linked_list_t *list, void *data);
linked_node_t *linked_list_insert(linked_node_t *before, void *data);
void linked_list_remove(linked_node_t *node);
