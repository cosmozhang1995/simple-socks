#ifndef _SS_RBTREE_H_
#define _SS_RBTREE_H_

#include "common/ss_types.h"
#include "common/ss_defs.h"
#include "util/ss_rbtree_def.h"

#define ss_rbtree_node_init(node, k, d)                                      \
    (node)->key = k;                                                         \
    (node)->data = d;

#define ss_rbtree_init(tree, s, c)                                           \
    ss_rbtree_sentinel_init(s);                                              \
    (tree)->root = s;                                                        \
    (tree)->sentinel = s;                                                    \
    (tree)->comparator = c

#define ss_rbtree_data(node, type, link)                                     \
    (type *) ((u_char *) (node) - offsetof(type, link))


void ss_rbtree_insert(ss_rbtree_t *tree, ss_rbtree_node_t *node);
void ss_rbtree_delete(ss_rbtree_t *tree, ss_rbtree_node_t *node);
void ss_rbtree_insert_value(ss_rbtree_node_t *root, ss_rbtree_node_t *node,
    ss_rbtree_node_t *sentinel);
void ss_rbtree_insert_timer_value(ss_rbtree_node_t *root,
    ss_rbtree_node_t *node, ss_rbtree_node_t *sentinel);
ss_rbtree_node_t *ss_rbtree_next(ss_rbtree_t *tree, ss_rbtree_node_t *node);
ss_rbtree_node_t *ss_rbtree_lookup(ss_rbtree_t *tree, ss_rbtree_node_t *target);


#define ss_rbt_red(node)               ((node)->color = 1)
#define ss_rbt_black(node)             ((node)->color = 0)
#define ss_rbt_is_red(node)            ((node)->color)
#define ss_rbt_is_black(node)          (!ss_rbt_is_red(node))
#define ss_rbt_copy_color(n1, n2)      (n1->color = n2->color)


/* a sentinel must be black */

#define ss_rbtree_sentinel_init(node)  ss_rbt_black(node)


static ss_inline ss_rbtree_node_t *
ss_rbtree_min(ss_rbtree_node_t *node, ss_rbtree_node_t *sentinel)
{
    while (node->left != sentinel) {
        node = node->left;
    }

    return node;
}


#endif
