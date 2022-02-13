#ifndef _SS_STRMAP_DEF_H_
#define _SS_STRMAP_DEF_H_

#include "util/ss_rbtree_def.h"

typedef struct {
    ss_rbtree_t tree;
    ss_rbtree_node_t sentinel;
} ss_strmap_t;

#endif
