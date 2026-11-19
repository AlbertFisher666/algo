/* ============================================================
 * tree_algo_test.cpp
 * Tests for tree_algo. assert-based, no frameworks.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/tree_algo.hpp"

#include <assert.h>
#include <stdio.h>

/* ============================================================
 * Validate
 * ============================================================ */

static void test_validate_empty(void) {
    Tree t; tree_init(&t);
    assert(tree_validate(&t) == 1);
    tree_free(&t);
}

static void test_validate_good(void) {
    Tree t; tree_init(&t);
    int ks[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) tree_insert(&t, ks[i]);
    assert(tree_validate(&t) == 1);
    tree_free(&t);
}

static void test_validate_broken(void) {
    Tree t; tree_init(&t);
    TreeNode* r = new TreeNode; r->key = 5;
    TreeNode* l = new TreeNode; l->key = 9;  /* wrong: > parent */
    r->left = l; r->right = NULL;
    l->left = l->right = NULL;
    t.root = r; t.size = 2;
    assert(tree_validate(&t) == 0);
    tree_free(&t);
}

/* ============================================================
 * Min / max
 * ============================================================ */

static void test_min_max_empty(void) {
    Tree t; tree_init(&t);
    int v = 0;
    assert(tree_min(&t, &v) == 0);
    assert(tree_max(&t, &v) == 0);
    tree_free(&t);
}

static void test_min_max(void) {
    Tree t; tree_init(&t);
    int ks[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) tree_insert(&t, ks[i]);
    int v = 0;
    assert(tree_min(&t, &v) == 1 && v == 1);
    assert(tree_max(&t, &v) == 1 && v == 9);
    tree_free(&t);
}

/* ============================================================
 * to_array_inorder
 * ============================================================ */

static void test_to_array_inorder(void) {
    Tree t; tree_init(&t);
    int ks[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) tree_insert(&t, ks[i]);
    int buf[7];
    int n = tree_to_array_inorder(&t, buf, 7);
    assert(n == 7);
    int expect[] = {1,3,4,5,7,8,9};
    for (int i = 0; i < 7; i++) assert(buf[i] == expect[i]);
    tree_free(&t);
}

/* ============================================================
 * from_sorted_array
 * ============================================================ */

static void test_from_sorted_array(void) {
    int ks[] = {1,2,3,4,5,6,7};
    Tree t; tree_init(&t);
    assert(tree_from_sorted_array(&t, ks, 7) == 0);
    assert(t.size == 7);
    assert(tree_validate(&t) == 1);
    /* perfect balance for 7 nodes: height 3 */
    assert(tree_height(&t) == 3);
    tree_free(&t);
}

/* ============================================================
 * Balance
 * ============================================================ */

static void test_balance(void) {
    Tree t; tree_init(&t);
    for (int i = 1; i <= 7; i++) tree_insert(&t, i);  /* degenerate */
    assert(tree_height(&t) == 7);
    assert(tree_balance(&t) == 0);
    assert(tree_validate(&t) == 1);
    assert(tree_height(&t) == 3);
    assert(t.size == 7);
    tree_free(&t);
}

/* ============================================================
 * BFS
 * ============================================================ */

typedef struct { int keys[16]; int n; } Acc;

static void acc_cb(int key, void* user) {
    Acc* a = (Acc*)user;
    a->keys[a->n++] = key;
}

static void test_bfs_order(void) {
    Tree t; tree_init(&t);
    int ks[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) tree_insert(&t, ks[i]);
    Acc a; a.n = 0;
    int visited = tree_bfs(&t, acc_cb, &a);
    assert(visited == 7);
    int expect[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) assert(a.keys[i] == expect[i]);
    tree_free(&t);
}

/* ============================================================
 * main
 * ============================================================ */

int main(void) {
    printf(">>> test_validate_empty\n");     test_validate_empty();
    printf(">>> test_validate_good\n");      test_validate_good();
    printf(">>> test_validate_broken\n");    test_validate_broken();
    printf(">>> test_min_max_empty\n");      test_min_max_empty();
    printf(">>> test_min_max\n");            test_min_max();
    printf(">>> test_to_array_inorder\n");   test_to_array_inorder();
    printf(">>> test_from_sorted_array\n");  test_from_sorted_array();
    printf(">>> test_balance\n");            test_balance();
    printf(">>> test_bfs_order\n");          test_bfs_order();
    printf("all tree_algo tests passed\n");
    return 0;
}
