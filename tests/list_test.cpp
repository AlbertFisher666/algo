/* ============================================================
 * list_test.cpp
 * Test suite for the doubly linked list.
 * Style: assert-based, no frameworks.
 * ============================================================ */

#include "algo/list.hpp"

#include <assert.h>
#include <stdio.h>

static void test_init_empty(void) {
    List l;
    list_init(&l);
    assert(l.head == NULL);
    assert(l.tail == NULL);
    assert(l.size == 0);
}

static void test_push_back(void) {
    List l;
    list_init(&l);
    assert(list_push_back(&l, 1) == 0);
    assert(list_push_back(&l, 2) == 0);
    assert(list_push_back(&l, 3) == 0);
    assert(l.size == 3);
    assert(l.head->key == 1);
    assert(l.tail->key == 3);
    assert(l.head->next->key == 2);
    assert(l.tail->prev->key == 2);
    list_free(&l);
}

static void test_push_front(void) {
    List l;
    list_init(&l);
    assert(list_push_front(&l, 1) == 0);
    assert(list_push_front(&l, 2) == 0);
    assert(list_push_front(&l, 3) == 0);
    assert(l.head->key == 3);
    assert(l.tail->key == 1);
    list_free(&l);
}

static void test_find(void) {
    List l;
    list_init(&l);
    list_push_back(&l, 5);
    list_push_back(&l, 7);
    assert(list_find(&l, 5) != NULL);
    assert(list_find(&l, 7) != NULL);
    assert(list_find(&l, 9) == NULL);
    list_free(&l);
}

static void test_remove(void) {
    List l;
    list_init(&l);
    list_push_back(&l, 1);
    list_push_back(&l, 2);
    list_push_back(&l, 3);
    assert(list_remove(&l, 2) == 0);
    assert(l.size == 2);
    assert(list_find(&l, 2) == NULL);
    assert(list_remove(&l, 2) == -1);
    assert(list_remove(&l, 1) == 0);  /* head */
    assert(l.head->key == 3);
    assert(list_remove(&l, 3) == 0);  /* tail */
    assert(l.head == NULL && l.tail == NULL);
    list_free(&l);
}

int main(void) {
    printf(">>> test_init_empty\n");  test_init_empty();
    printf(">>> test_push_back\n");   test_push_back();
    printf(">>> test_push_front\n");  test_push_front();
    printf(">>> test_find\n");        test_find();
    printf(">>> test_remove\n");      test_remove();
    printf("all list tests passed\n");
    return 0;
}
