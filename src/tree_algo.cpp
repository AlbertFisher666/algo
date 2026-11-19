/* ============================================================
 * tree_algo.cpp
 * Algorithms over the BST. Integer keys.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/tree_algo.hpp"

#include <stdio.h>
#include <stddef.h>

/* ============================================================
 * Validate
 * ============================================================ */

/* Open bounds just outside int range.  Strict < / > means
 * duplicates are rejected: a repeated key cannot be > lo and
 * < hi at the same time on the same side of its parent. */
static int validate_node(const TreeNode* n, long long lo, long long hi) {
    if (n == NULL) return 1;
    if ((long long)n->key <= lo || (long long)n->key >= hi) return 0;
    if (!validate_node(n->left,  lo, (long long)n->key)) return 0;
    if (!validate_node(n->right, (long long)n->key, hi)) return 0;
    return 1;
}

int tree_validate(const Tree* t) {
    if (t == NULL) return 0;

    /* Use values just outside int range as open bounds. */
    long long lo = -2147483648LL - 1;
    long long hi =  2147483647LL + 1;
    return validate_node(t->root, lo, hi);
}

/* ============================================================
 * Min / max
 * ============================================================ */

int tree_min(const Tree* t, int* out) {
    if (t == NULL || out == NULL || t->root == NULL) return 0;

    const TreeNode* cur = t->root;
    while (cur->left != NULL) cur = cur->left;
    *out = cur->key;
    return 1;
}

int tree_max(const Tree* t, int* out) {
    if (t == NULL || out == NULL || t->root == NULL) return 0;

    const TreeNode* cur = t->root;
    while (cur->right != NULL) cur = cur->right;
    *out = cur->key;
    return 1;
}

/* ============================================================
 * Array <-> tree
 * ============================================================ */

static void fill_inorder(const TreeNode* n, int* buf, int* pos) {
    if (n == NULL) return;

    fill_inorder(n->left, buf, pos);
    buf[(*pos)++] = n->key;
    fill_inorder(n->right, buf, pos);
}

int tree_to_array_inorder(const Tree* t, int* buf, int cap) {
    if (t == NULL || (t->size > 0 && buf == NULL)) return -1;
    if (cap < t->size) return -1;

    int pos = 0;
    fill_inorder(t->root, buf, &pos);
    return pos;
}

/* Build a balanced subtree from a sorted slice a[lo..hi].
 * mid = lo + (hi - lo) / 2 keeps recursion depth at
 * O(log n) and the result perfectly balanced. */
static TreeNode* build_balanced(const int* a, int lo, int hi) {
    if (lo > hi) return NULL;

    int mid = lo + (hi - lo) / 2;
    TreeNode* n = new TreeNode;
    n->key   = a[mid];
    n->left  = build_balanced(a, lo, mid - 1);
    n->right = build_balanced(a, mid + 1, hi);
    return n;
}

int tree_from_sorted_array(Tree* t, const int* keys, int n) {
    if (t == NULL) return -1;
    if (n < 0) return -1;
    if (n > 0 && keys == NULL) return -1;
    if (t->root != NULL || t->size != 0) return -1; /* must be empty */

    t->root = build_balanced(keys, 0, n - 1);
    t->size = n;
    return 0;
}

/* ============================================================
 * Balance
 * ============================================================ */

/* In-order -> array -> tree_free -> from_sorted_array.
 * Same key set, same BST invariant, O(n) time, O(n) space. */
int tree_balance(Tree* t) {
    if (t == NULL) return -1;
    if (t->size == 0) return 0;

    int* buf = new int[t->size];
    int n = tree_to_array_inorder(t, buf, t->size);
    if (n != t->size) {
        delete[] buf;
        return -1;
    }

    tree_free(t);
    int rc = tree_from_sorted_array(t, buf, n);
    delete[] buf;
    return rc;
}

/* ============================================================
 * BFS
 * ============================================================ */

/* Array-backed queue, head/tail indices, no std::queue.
 * Capacity t->size is exact: every node is enqueued once. */
int tree_bfs(const Tree* t, TreeVisitFn cb, void* user) {
    if (t == NULL || cb == NULL || t->root == NULL) return 0;

    const TreeNode** q = new const TreeNode*[t->size];
    int head = 0;
    int tail = 0;
    q[tail++] = t->root;

    int visited = 0;
    while (head < tail) {
        const TreeNode* n = q[head++];
        cb(n->key, user);
        visited++;

        if (n->left  != NULL) q[tail++] = n->left;
        if (n->right != NULL) q[tail++] = n->right;
    }

    delete[] q;
    return visited;
}
