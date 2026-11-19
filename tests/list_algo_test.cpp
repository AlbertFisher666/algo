/* ============================================================
 * list_algo_test.cpp
 * Tests for list_algo. assert-based, no frameworks.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/list_algo.hpp"

#include <assert.h>
#include <stdio.h>

/* ============================================================
 * Helpers
 * ============================================================ */

/* Build 1..n, return list. */
static void build(List* l, int n)
{
    list_init(l);
    for (int i = 1; i <= n; i++)
        list_push_back(l, i);
}

/* Walk forward, assert keys and prev-chain consistency. */
static void check_forward(List* l, const int* expect, int n)
{
    ListNode* cur;
    ListNode* prev;

    assert(l->size == n);
    cur = l->head;
    prev = NULL;
    for (int i = 0; i < n; i++) {
        assert(cur != NULL);
        assert(cur->key == expect[i]);
        assert(cur->prev == prev);
        prev = cur;
        cur = cur->next;
    }
    assert(cur == NULL);
    assert(l->tail == prev);
}

/* ============================================================
 * Reverse
 * ============================================================ */

static void test_reverse_empty(void)
{
    List l; list_init(&l);
    assert(list_reverse(&l) == 0);
    assert(l.head == NULL && l.tail == NULL && l.size == 0);
    list_free(&l);
}

static void test_reverse_one(void)
{
    List l; build(&l, 1);
    assert(list_reverse(&l) == 0);
    int e[] = {1}; check_forward(&l, e, 1);
    list_free(&l);
}

static void test_reverse_even(void)
{
    List l; build(&l, 4);
    assert(list_reverse(&l) == 0);
    int e[] = {4,3,2,1}; check_forward(&l, e, 4);
    list_free(&l);
}

static void test_reverse_odd(void)
{
    List l; build(&l, 5);
    assert(list_reverse(&l) == 0);
    int e[] = {5,4,3,2,1}; check_forward(&l, e, 5);
    list_free(&l);
}

/* ============================================================
 * Middle
 * ============================================================ */

static void test_middle_empty(void)
{
    List l; list_init(&l);
    assert(list_middle(&l) == NULL);
    list_free(&l);
}

static void test_middle_one(void)
{
    List l; build(&l, 1);
    assert(list_middle(&l)->key == 1);
    list_free(&l);
}

static void test_middle_odd(void)
{
    List l; build(&l, 5);
    assert(list_middle(&l)->key == 3);  /* lower middle for 1..5 */
    list_free(&l);
}

static void test_middle_even(void)
{
    List l; build(&l, 4);
    assert(list_middle(&l)->key == 2);  /* lower middle for 1..4 */
    list_free(&l);
}

/* ============================================================
 * Sort
 * ============================================================ */

static void test_sort_empty(void)
{
    List l; list_init(&l);
    assert(list_sort(&l) == 0);
    assert(l.head == NULL && l.size == 0);
    list_free(&l);
}

static void test_sort_one(void)
{
    List l; build(&l, 1);
    assert(list_sort(&l) == 0);
    int e[] = {1}; check_forward(&l, e, 1);
    list_free(&l);
}

static void test_sort_reverse_input(void)
{
    List l; list_init(&l);
    for (int i = 10; i >= 1; i--)
        list_push_back(&l, i);
    assert(list_sort(&l) == 0);
    int e[10]; for (int i = 0; i < 10; i++) e[i] = i + 1;
    check_forward(&l, e, 10);
    list_free(&l);
}

static void test_sort_sorted_input(void)
{
    List l; build(&l, 6);
    assert(list_sort(&l) == 0);
    int e[] = {1,2,3,4,5,6}; check_forward(&l, e, 6);
    list_free(&l);
}

static void test_sort_duplicates(void)
{
    List l; list_init(&l);
    int in[] = {3,1,3,2,1,3};
    for (int i = 0; i < 6; i++)
        list_push_back(&l, in[i]);
    assert(list_sort(&l) == 0);
    int e[] = {1,1,2,3,3,3}; check_forward(&l, e, 6);
    list_free(&l);
}

/* ============================================================
 * main
 * ============================================================ */

int main(void)
{
    printf(">>> test_reverse_empty\n");      test_reverse_empty();
    printf(">>> test_reverse_one\n");        test_reverse_one();
    printf(">>> test_reverse_even\n");       test_reverse_even();
    printf(">>> test_reverse_odd\n");        test_reverse_odd();
    printf(">>> test_middle_empty\n");       test_middle_empty();
    printf(">>> test_middle_one\n");         test_middle_one();
    printf(">>> test_middle_odd\n");         test_middle_odd();
    printf(">>> test_middle_even\n");        test_middle_even();
    printf(">>> test_sort_empty\n");         test_sort_empty();
    printf(">>> test_sort_one\n");           test_sort_one();
    printf(">>> test_sort_reverse_input\n"); test_sort_reverse_input();
    printf(">>> test_sort_sorted_input\n");  test_sort_sorted_input();
    printf(">>> test_sort_duplicates\n");    test_sort_duplicates();
    printf("all list_algo tests passed\n");
    return 0;
}
