/* ============================================================
 * hash_test.cpp
 * Test suite for the chained hash table.
 * Style: assert-based, no frameworks.
 * ============================================================ */

#include "algo/hash.hpp"

#include <assert.h>
#include <stdio.h>

static void test_init_bad_capacity(void) {
    Hash h;
    assert(hash_init(&h, 0)  == -1);
    assert(hash_init(&h, -1) == -1);
}

static void test_insert_find(void) {
    Hash h;
    assert(hash_init(&h, 16) == 0);
    assert(hash_insert(&h, 5) == 0);
    assert(hash_insert(&h, 7) == 0);
    assert(hash_insert(&h, 5) == -1);  /* duplicate */
    assert(h.size == 2);
    assert(hash_find(&h, 5) == 1);
    assert(hash_find(&h, 7) == 1);
    assert(hash_find(&h, 9) == 0);
    hash_free(&h);
}

static void test_remove(void) {
    Hash h;
    assert(hash_init(&h, 8) == 0);
    hash_insert(&h, 10);
    hash_insert(&h, 20);
    assert(hash_remove(&h, 10) == 0);
    assert(hash_find(&h, 10) == 0);
    assert(hash_find(&h, 20) == 1);
    assert(hash_remove(&h, 10) == -1);
    assert(h.size == 1);
    hash_free(&h);
}

static void test_collision(void) {
    Hash h;
    assert(hash_init(&h, 1) == 0);  /* force collision */
    assert(hash_insert(&h, 1) == 0);
    assert(hash_insert(&h, 2) == 0);
    assert(hash_insert(&h, 3) == 0);
    assert(h.size == 3);
    assert(hash_find(&h, 1) == 1);
    assert(hash_find(&h, 2) == 1);
    assert(hash_find(&h, 3) == 1);
    hash_free(&h);
}

int main(void) {
    printf(">>> test_init_bad_capacity\n"); test_init_bad_capacity();
    printf(">>> test_insert_find\n");       test_insert_find();
    printf(">>> test_remove\n");            test_remove();
    printf(">>> test_collision\n");         test_collision();
    printf("all hash tests passed\n");
    return 0;
}
