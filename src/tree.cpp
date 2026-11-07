/* ============================================================
 * tree.cpp
 * Binary search tree. Integer keys. Iterative insert.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/tree.hpp"

#include <stdio.h>

/* ============================================================
 * Lifecycle
 * ============================================================ */

void tree_init(Tree* t) {
    t->root = NULL;
    t->size = 0;
}

static void tree_free_node(TreeNode* n) {
    if (n == NULL) return;
    tree_free_node(n->left);   /* post-order: */
    tree_free_node(n->right);  /* children first, */
    delete n;                  /* then node */
}

void tree_free(Tree* t) {
    tree_free_node(t->root);
    t->root = NULL;
    t->size = 0;
}

int tree_find(const Tree* t, int key) {
    const TreeNode* cur = t->root;
    while (cur != NULL) {
        if (key == cur->key) {
            return 1;
        }
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return 0;
}

static int tree_height_node(const TreeNode* n) {
    if (n == NULL) return 0;
    int l = tree_height_node(n->left);
    int r = tree_height_node(n->right);
    return 1 + (l > r ? l : r);
}

int tree_height(const Tree* t) {
    return tree_height_node(t->root);
}

/* helper: detach the minimum node of subtree n; returns its key, frees node */
static int tree_detach_min(TreeNode** slot) {
    while ((*slot)->left != NULL) {
        slot = &(*slot)->left;
    }
    TreeNode* victim = *slot;
    int key = victim->key;
    *slot = victim->right;
    delete victim;
    return key;
}

int tree_remove(Tree* t, int key) {
    TreeNode** slot = &t->root;
    while (*slot != NULL && (*slot)->key != key) {
        slot = (key < (*slot)->key) ? &(*slot)->left : &(*slot)->right;
    }
    if (*slot == NULL) {
        return -1;
    }
    TreeNode* victim = *slot;
    if (victim->left == NULL) {
        *slot = victim->right;
        delete victim;
    } else if (victim->right == NULL) {
        *slot = victim->left;
        delete victim;
    } else {
        victim->key = tree_detach_min(&victim->right);
    }
    t->size--;
    return 0;
}

static void tree_print_node(const TreeNode* n) {
    if (n == NULL) return;
    tree_print_node(n->left);
    printf("%d\n", n->key);
    tree_print_node(n->right);
}

void tree_print(const Tree* t) {
    tree_print_node(t->root);
}

/* ============================================================
 * Insert
 * ============================================================ */

int tree_insert(Tree* t, int key) {
    TreeNode** slot = &t->root;
    while (*slot != NULL) {
        if (key == (*slot)->key) {
            return -1;  /* duplicate */
        }
        slot = (key < (*slot)->key) ? &(*slot)->left : &(*slot)->right;
    }
    *slot = new TreeNode;
    (*slot)->key   = key;
    (*slot)->left  = NULL;
    (*slot)->right = NULL;
    t->size++;
    return 0;
}
