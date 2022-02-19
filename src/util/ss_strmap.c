#include "util/ss_strmap.h"

#include <string.h>
#include <stdlib.h>

#include "util/ss_rbtree.h"
#include "util/ss_string_utils.h"

typedef struct {
    ss_rbtree_node_t         node;
    char                    *key;
    ss_variable_t            value;
} ss_strmap_node_t;

// static void ss_str_rbtree_insert_value(ss_rbtree_node_t *temp,
//     ss_rbtree_node_t *node, ss_rbtree_node_t *sentinel);

// static ss_strmap_node_t *ss_str_rbtree_lookup(ss_rbtree_t *rbtree, char *name, ss_uint32_t hash);

static int ss_strmap_comparator(ss_rbtree_node_t *node1, ss_rbtree_node_t *node2);

void ss_strmap_initialize(ss_strmap_t *map)
{
    // ss_rbtree_init(&map->tree, &map->sentinel, ss_str_rbtree_insert_value);
    ss_rbtree_init(&map->tree, &map->sentinel, ss_strmap_comparator);
}
void ss_strmap_uninitialize(ss_strmap_t *map)
{
}

// static void ss_str_rbtree_insert_value(ss_rbtree_node_t *temp,
//     ss_rbtree_node_t *node, ss_rbtree_node_t *sentinel)
// {
//     ss_strmap_node_t      *n, *t;
//     ss_rbtree_node_t     **p;

//     for ( ;; ) {
//         n = (ss_strmap_node_t *) node;
//         t = (ss_strmap_node_t *) temp;

//         if (node->key != temp->key) {
//             p = (node->key < temp->key) ? &temp->left : &temp->right;
//         } else {
//             p = (strcmp(n->str.data, t->str.data, n->str.len) < 0)
//                  ? &temp->left : &temp->right;
//         }

//         if (*p == sentinel) {
//             break;
//         }

//         temp = *p;
//     }

//     *p = node;
//     node->parent = temp;
//     node->left = sentinel;
//     node->right = sentinel;
//     ngx_rbt_red(node);
// }

// static ss_strmap_node_t *ss_str_rbtree_lookup(ss_rbtree_t *rbtree, char *name,
//     ss_uint32_t hash)
// {
// }



static void ss_strmap_node_initialize(ss_strmap_node_t *node, const char *key)
{
    ss_uint32_t hashkey;
    size_t keylen;
    hashkey = ss_string_hash(key);
    ss_rbtree_node_init(&node->node, hashkey, 0);
    keylen = strlen(key);
    node->key = malloc(keylen + 1);
    strcpy(node->key, key);
}

static ss_strmap_node_t *ss_strmap_node_create(const char *key, ss_variable_t value)
{
    ss_strmap_node_t *node;
    node = malloc(sizeof(ss_strmap_node_t));
    ss_strmap_node_initialize(node, key);
    node->value = value;
    return node;
}

static void ss_strmap_node_uninitialize(ss_strmap_node_t *node)
{
    if (node->key) free(node->key);
}

static void ss_strmap_node_release(ss_strmap_node_t *node)
{
    ss_strmap_node_uninitialize(node);
    free(node);
}

static int ss_strmap_comparator(ss_rbtree_node_t *node1, ss_rbtree_node_t *node2)
{
    ss_strmap_node_t *n1, *n2;
    n1 = (ss_strmap_node_t *)node1;
    n2 = (ss_strmap_node_t *)node2;
    return strcmp(n1->key, n2->key);
}

static ss_inline ss_strmap_node_t *ss_strmap_lookup(ss_strmap_t *map, const char *key)
{
    ss_strmap_node_t  pseudo_node;
    ss_uint32_t       hashkey;
    
    hashkey = ss_string_hash(key);
    ss_rbtree_node_init(&pseudo_node.node, hashkey, 0);
    pseudo_node.key = (char *)key;
    return (ss_strmap_node_t *) ss_rbtree_lookup(&map->tree, &pseudo_node.node);
}

ss_bool_t ss_strmap_get(ss_strmap_t *map, const char *key, ss_variable_t *value)
{
    ss_strmap_node_t *node;
    ss_uint32_t       hashkey;

    node = ss_strmap_lookup(map, key);
    if (node) {
        if (value) *value = node->value;
        return SS_TRUE;
    } else {
        return SS_FALSE;
    }
}

ss_bool_t ss_strmap_put(ss_strmap_t *map, const char *key, ss_variable_t value, ss_variable_t *erased_value)
{
    ss_strmap_node_t *node;

    node = ss_strmap_lookup(map, key);
    if (node) {
        if (erased_value) *erased_value = node->value;
        node->value = value;
        return SS_TRUE;
    } else {
        node = ss_strmap_node_create(key, value);
        ss_rbtree_insert(&map->tree, (ss_rbtree_node_t *)node);
        return SS_FALSE;
    }
}

ss_bool_t ss_strmap_erase(ss_strmap_t *map, const char *key, ss_variable_t *erased_value)
{
    ss_strmap_node_t *node;

    node = ss_strmap_lookup(map, key);
    if (node) {
        if (erased_value) *erased_value = node->value;
        ss_rbtree_delete(&map->tree, (ss_rbtree_node_t *)node);
        ss_strmap_node_release(node);
        return SS_TRUE;
    } else {
        return SS_FALSE;
    }
}

static void ss_strmap_foreach_inner(ss_rbtree_t *tree, ss_rbtree_node_t *node,
    ss_strmap_visitor_t function, void *argument)
{
    ss_strmap_node_t *mnode;
    if (node == SS_NULL || node == tree->sentinel) return;
    mnode = (ss_strmap_node_t *)node;
    ss_strmap_foreach_inner(tree, mnode->node.left, function, argument);
    if (function) function(mnode->key, &mnode->value, argument);
    ss_strmap_foreach_inner(tree, mnode->node.right, function, argument);
}

void ss_strmap_foreach(ss_strmap_t *map, ss_strmap_visitor_t function, void *argument)
{
    ss_strmap_foreach_inner(&map->tree, map->tree.root, function, argument);
}

static void ss_strmap_release_inner(ss_rbtree_t *tree, ss_rbtree_node_t *node,
    ss_strmap_visitor_t release_function, void *argument)
{
    ss_strmap_node_t *mnode;
    if (node == SS_NULL || node == tree->sentinel) return;
    mnode = (ss_strmap_node_t *)node;
    ss_strmap_release_inner(tree, mnode->node.left, release_function, argument);
    ss_strmap_release_inner(tree, mnode->node.right, release_function, argument);
    if (release_function) release_function(mnode->key, &mnode->value, argument);
    ss_strmap_node_release(mnode);
}

void ss_strmap_clear(ss_strmap_t *map, ss_strmap_visitor_t release_function, void *argument)
{
    ss_strmap_release_inner(&map->tree, map->tree.root, release_function, argument);
    ss_rbtree_init(&map->tree, &map->sentinel, ss_strmap_comparator);
}
