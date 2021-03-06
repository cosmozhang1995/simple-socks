#include "util/ss_rbtree.h"

/*
 * The red-black tree code is based on the algorithm described in
 * the "Introduction to Algorithms" by Cormen, Leiserson and Rivest.
 */


static ss_inline void ss_rbtree_left_rotate(ss_rbtree_node_t **root,
    ss_rbtree_node_t *sentinel, ss_rbtree_node_t *node);
static ss_inline void ss_rbtree_right_rotate(ss_rbtree_node_t **root,
    ss_rbtree_node_t *sentinel, ss_rbtree_node_t *node);


void
ss_rbtree_insert(ss_rbtree_t *tree, ss_rbtree_node_t *node)
{
    ss_rbtree_node_t  **root, *temp, *sentinel, **p;

    /* a binary tree insert */

    root = &tree->root;
    sentinel = tree->sentinel;

    if (*root == sentinel) {
        node->parent = SS_NULL;
        node->left = sentinel;
        node->right = sentinel;
        ss_rbt_black(node);
        *root = node;

        return;
    }

    // tree->insert(*root, node, sentinel);

    temp = *root;
    for ( ;; ) {
        if (node->key != temp->key) {
            p = (node->key < temp->key) ? &temp->left : &temp->right;
        } else if (tree->comparator) {
            p = (tree->comparator(node, temp) < 0) ? &temp->left : &temp->right;
        } else {
            p = &temp->right;
        }
        if (*p == sentinel) {
            break;
        }
        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ss_rbt_red(node);

    /* re-balance tree */

    while (node != *root && ss_rbt_is_red(node->parent)) {

        if (node->parent == node->parent->parent->left) {
            temp = node->parent->parent->right;

            if (ss_rbt_is_red(temp)) {
                ss_rbt_black(node->parent);
                ss_rbt_black(temp);
                ss_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    ss_rbtree_left_rotate(root, sentinel, node);
                }

                ss_rbt_black(node->parent);
                ss_rbt_red(node->parent->parent);
                ss_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }

        } else {
            temp = node->parent->parent->left;

            if (ss_rbt_is_red(temp)) {
                ss_rbt_black(node->parent);
                ss_rbt_black(temp);
                ss_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    ss_rbtree_right_rotate(root, sentinel, node);
                }

                ss_rbt_black(node->parent);
                ss_rbt_red(node->parent->parent);
                ss_rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }

    ss_rbt_black(*root);
}


void
ss_rbtree_insert_value(ss_rbtree_node_t *temp, ss_rbtree_node_t *node,
    ss_rbtree_node_t *sentinel)
{
    ss_rbtree_node_t  **p;

    for ( ;; ) {

        p = (node->key < temp->key) ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ss_rbt_red(node);
}


void
ss_rbtree_insert_timer_value(ss_rbtree_node_t *temp, ss_rbtree_node_t *node,
    ss_rbtree_node_t *sentinel)
{
    ss_rbtree_node_t  **p;

    for ( ;; ) {

        /*
         * Timer values
         * 1) are spread in small range, usually several minutes,
         * 2) and overflow each 49 days, if milliseconds are stored in 32 bits.
         * The comparison takes into account that overflow.
         */

        /*  node->key < temp->key */

        p = ((ss_rbtree_key_int_t) (node->key - temp->key) < 0)
            ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ss_rbt_red(node);
}


void
ss_rbtree_delete(ss_rbtree_t *tree, ss_rbtree_node_t *node)
{
    ss_uint32_t         red;
    ss_rbtree_node_t  **root, *sentinel, *subst, *temp, *w;

    /* a binary tree delete */

    root = &tree->root;
    sentinel = tree->sentinel;

    if (node->left == sentinel) {
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) {
        temp = node->left;
        subst = node;

    } else {
        subst = ss_rbtree_min(node->right, sentinel);
        temp = subst->right;
    }

    if (subst == *root) {
        *root = temp;
        ss_rbt_black(temp);

        /* DEBUG stuff */
        node->left = SS_NULL;
        node->right = SS_NULL;
        node->parent = SS_NULL;
        node->key = 0;

        return;
    }

    red = ss_rbt_is_red(subst);

    if (subst == subst->parent->left) {
        subst->parent->left = temp;

    } else {
        subst->parent->right = temp;
    }

    if (subst == node) {

        temp->parent = subst->parent;

    } else {

        if (subst->parent == node) {
            temp->parent = subst;

        } else {
            temp->parent = subst->parent;
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        ss_rbt_copy_color(subst, node);

        if (node == *root) {
            *root = subst;

        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;
            } else {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    /* DEBUG stuff */
    node->left = SS_NULL;
    node->right = SS_NULL;
    node->parent = SS_NULL;
    node->key = 0;

    if (red) {
        return;
    }

    /* a delete fixup */

    while (temp != *root && ss_rbt_is_black(temp)) {

        if (temp == temp->parent->left) {
            w = temp->parent->right;

            if (ss_rbt_is_red(w)) {
                ss_rbt_black(w);
                ss_rbt_red(temp->parent);
                ss_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            if (ss_rbt_is_black(w->left) && ss_rbt_is_black(w->right)) {
                ss_rbt_red(w);
                temp = temp->parent;

            } else {
                if (ss_rbt_is_black(w->right)) {
                    ss_rbt_black(w->left);
                    ss_rbt_red(w);
                    ss_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                ss_rbt_copy_color(w, temp->parent);
                ss_rbt_black(temp->parent);
                ss_rbt_black(w->right);
                ss_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (ss_rbt_is_red(w)) {
                ss_rbt_black(w);
                ss_rbt_red(temp->parent);
                ss_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (ss_rbt_is_black(w->left) && ss_rbt_is_black(w->right)) {
                ss_rbt_red(w);
                temp = temp->parent;

            } else {
                if (ss_rbt_is_black(w->left)) {
                    ss_rbt_black(w->right);
                    ss_rbt_red(w);
                    ss_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                ss_rbt_copy_color(w, temp->parent);
                ss_rbt_black(temp->parent);
                ss_rbt_black(w->left);
                ss_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    ss_rbt_black(temp);
}

ss_rbtree_node_t *
ss_rbtree_lookup(ss_rbtree_t *tree, ss_rbtree_node_t *target)
{
    ss_rbtree_node_t           *node, *sentinel;
    ss_rbtree_node_comparator_t comparator;
    int cmp;

    node = tree->root;
    sentinel = tree->sentinel;
    comparator = tree->comparator;

    while (node != sentinel) {
        if (target->key != node->key) {
            node = (target->key < node->key) ? node->left : node->right;
            continue;
        }
        if (comparator && (cmp = comparator(target, node)) != 0) {
            node = cmp < 0 ? node->left : node->right;
            continue;
        }
        return node;
    }

    return SS_NULL;
}


static ss_inline void
ss_rbtree_left_rotate(ss_rbtree_node_t **root, ss_rbtree_node_t *sentinel,
    ss_rbtree_node_t *node)
{
    ss_rbtree_node_t  *temp;

    temp = node->right;
    node->right = temp->left;

    if (temp->left != sentinel) {
        temp->left->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->left) {
        node->parent->left = temp;

    } else {
        node->parent->right = temp;
    }

    temp->left = node;
    node->parent = temp;
}


static ss_inline void
ss_rbtree_right_rotate(ss_rbtree_node_t **root, ss_rbtree_node_t *sentinel,
    ss_rbtree_node_t *node)
{
    ss_rbtree_node_t  *temp;

    temp = node->left;
    node->left = temp->right;

    if (temp->right != sentinel) {
        temp->right->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->right) {
        node->parent->right = temp;

    } else {
        node->parent->left = temp;
    }

    temp->right = node;
    node->parent = temp;
}


ss_rbtree_node_t *
ss_rbtree_next(ss_rbtree_t *tree, ss_rbtree_node_t *node)
{
    ss_rbtree_node_t  *root, *sentinel, *parent;

    sentinel = tree->sentinel;

    if (node->right != sentinel) {
        return ss_rbtree_min(node->right, sentinel);
    }

    root = tree->root;

    for ( ;; ) {
        parent = node->parent;

        if (node == root) {
            return SS_NULL;
        }

        if (node == parent->left) {
            return parent;
        }

        node = parent;
    }
}
