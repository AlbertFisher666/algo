/* ============================================================
 * tree_test.cpp
 * Test suite for the binary search tree.
 *
 * Style: assert-based, no frameworks.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/tree.hpp"

#include <assert.h>
#include <stdio.h>

/* ============================================================
 * Section 1: insert
 * ============================================================ */

static void test_insert_empty(void) {
    Tree t;
    tree_init(&t);

    assert(t.root == NULL);
    assert(t.size == 0);

    assert(tree_insert(&t, 5) == 0);
    assert(t.size == 1);
    assert(t.root != NULL);
    assert(t.root->key == 5);
    assert(t.root->left == NULL);
    assert(t.root->right == NULL);
}

static void test_insert_basic(void) {
    Tree t;
    tree_init(&t);

    assert(tree_insert(&t, 5) == 0);
    assert(tree_insert(&t, 3) == 0);
    assert(tree_insert(&t, 8) == 0);
    assert(tree_insert(&t, 1) == 0);
    assert(tree_insert(&t, 4) == 0);
    assert(tree_insert(&t, 7) == 0);
    assert(tree_insert(&t, 9) == 0);

    assert(t.size == 7);
    assert(t.root->key == 5);
    assert(t.root->left->key == 3);
    assert(t.root->right->key == 8);
    assert(t.root->left->left->key == 1);
    assert(t.root->left->right->key == 4);
    assert(t.root->right->left->key == 7);
    assert(t.root->right->right->key == 9);
}

static void test_insert_duplicate(void) {
    Tree t;
    tree_init(&t);

    assert(tree_insert(&t, 5) == 0);
    assert(tree_insert(&t, 5) == -1);  /* duplicate */
    assert(t.size == 1);
}

static void test_find(void) {
    Tree t;
    tree_init(&t);

    assert(tree_insert(&t, 5) == 0);
    assert(tree_insert(&t, 3) == 0);
    assert(tree_insert(&t, 8) == 0);
    assert(tree_insert(&t, 1) == 0);
    assert(tree_insert(&t, 4) == 0);
    assert(tree_insert(&t, 7) == 0);
    assert(tree_insert(&t, 9) == 0);

    assert(tree_find(&t, 5) == 1);
    assert(tree_find(&t, 1) == 1);
    assert(tree_find(&t, 9) == 1);
    assert(tree_find(&t, 4) == 1);

    assert(tree_find(&t, 6) == 0);
    assert(tree_find(&t, 0) == 0);
    assert(tree_find(&t, 10) == 0);

    tree_free(&t);
}

static void test_height(void) {
    Tree t;
    tree_init(&t);
    assert(tree_height(&t) == 0);
    tree_insert(&t, 5);
    assert(tree_height(&t) == 1);
    tree_insert(&t, 3);
    tree_insert(&t, 8);
    assert(tree_height(&t) == 2);
    tree_insert(&t, 1);
    tree_insert(&t, 4);
    tree_insert(&t, 7);
    tree_insert(&t, 9);
    assert(tree_height(&t) == 3);
    tree_free(&t);
}

static void test_remove_leaf(void) {
    Tree t;
    tree_init(&t);
    tree_insert(&t, 5);
    tree_insert(&t, 3);
    tree_insert(&t, 8);
    assert(tree_remove(&t, 3) == 0);
    assert(tree_find(&t, 3) == 0);
    assert(t.size == 2);
    assert(tree_remove(&t, 3) == -1);  /* not found */
    tree_free(&t);
}

static void test_remove_one_child(void) {
    Tree t;
    tree_init(&t);
    tree_insert(&t, 5);
    tree_insert(&t, 3);
    tree_insert(&t, 1);
    assert(tree_remove(&t, 3) == 0);  /* 3 has left child 1 */
    assert(tree_find(&t, 1) == 1);
    assert(tree_find(&t, 3) == 0);
    tree_free(&t);
}

static void test_remove_two_children(void) {
    Tree t;
    tree_init(&t);
    tree_insert(&t, 5);
    tree_insert(&t, 3);
    tree_insert(&t, 8);
    tree_insert(&t, 4);
    tree_insert(&t, 7);
    assert(tree_remove(&t, 5) == 0);  /* 5 has children 3, 8 */
    assert(tree_find(&t, 5) == 0);
    assert(tree_find(&t, 3) == 1);
    assert(tree_find(&t, 8) == 1);
    assert(tree_find(&t, 4) == 1);
    assert(tree_find(&t, 7) == 1);
    tree_free(&t);
}

static void test_print(void) {
    Tree t;
    tree_init(&t);
    tree_insert(&t, 5);
    tree_insert(&t, 3);
    tree_insert(&t, 8);
    printf("in-order: ");
    tree_print(&t);  /* 3 5 8 */
    tree_free(&t);
}

/* ============================================================
 * main
 * ============================================================ */

int main(void) {
    printf(">>> test_insert_empty\n");     test_insert_empty();
    printf(">>> test_insert_basic\n");     test_insert_basic();
    printf(">>> test_insert_duplicate\n"); test_insert_duplicate();
    printf(">>> test_find\n");       test_find();
    printf(">>> test_height\n");           test_height();
    printf(">>> test_remove_leaf\n");      test_remove_leaf();
    printf(">>> test_remove_one_child\n"); test_remove_one_child();
    printf(">>> test_remove_two_children\n"); test_remove_two_children();
    printf(">>> test_print\n");            test_print();

    printf("all tests passed\n");
    return 0;
}
