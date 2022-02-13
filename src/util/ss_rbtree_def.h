#ifndef _SS_RBTREE_DEF_H_
#define _SS_RBTREE_DEF_H_

#include "common/ss_types.h"

typedef ss_uint32_t  ss_rbtree_key_t;
typedef ss_int32_t   ss_rbtree_key_int_t;


typedef struct ss_rbtree_node_s ss_rbtree_node_t;

struct ss_rbtree_node_s {
    ss_rbtree_key_t       key;
    ss_rbtree_node_t     *left;
    ss_rbtree_node_t     *right;
    ss_rbtree_node_t     *parent;
    ss_uint8_t            color;
    ss_uint8_t            data;
};

// typedef void (*ss_rbtree_insert_pt) (ss_rbtree_node_t *root,
//     ss_rbtree_node_t *node, ss_rbtree_node_t *sentinel);

typedef int (*ss_rbtree_node_comparator_t) (ss_rbtree_node_t *node1, ss_rbtree_node_t *node2);

typedef struct ss_rbtree_s  ss_rbtree_t;

struct ss_rbtree_s {
    ss_rbtree_node_t           *root;
    ss_rbtree_node_t           *sentinel;
    // ss_rbtree_insert_pt         insert;
    ss_rbtree_node_comparator_t comparator;
};

#endif
