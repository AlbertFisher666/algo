/* ============================================================
 * hash_algo_test.cpp
 * Tests for hash_algo. assert-based, no frameworks.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/hash_algo.hpp"

#include <assert.h>
#include <stdio.h>

/* ============================================================
 * Empty
 * ============================================================ */

static void test_empty(void)
{
    Hash h;
    HashStats s;

    assert(hash_init(&h, 8) == 0);
    assert(hash_stats(&h, &s) == 0);
    assert(s.capacity == 8);
    assert(s.size == 0);
    assert(s.min_chain == 0);
    assert(s.max_chain == 0);
    assert(s.empty_buckets == 8);
    assert(s.load_factor == 0.0);
    hash_free(&h);
}

/* ============================================================
 * All in one bucket
 * ============================================================ */

static void test_all_in_one_bucket(void)
{
    Hash h;
    HashStats s;

    assert(hash_init(&h, 1) == 0);
    hash_insert(&h, 10);
    hash_insert(&h, 20);
    hash_insert(&h, 30);
    assert(hash_stats(&h, &s) == 0);
    assert(s.capacity == 1);
    assert(s.size == 3);
    assert(s.min_chain == 3);
    assert(s.max_chain == 3);
    assert(s.empty_buckets == 0);
    hash_free(&h);
}

/* ============================================================
 * Spread
 * ============================================================ */

static void test_spread(void)
{
    Hash h;
    HashStats s;
    int i;

    assert(hash_init(&h, 8) == 0);
    for (i = 0; i < 8; i++)
        hash_insert(&h, i);
    assert(hash_stats(&h, &s) == 0);
    assert(s.capacity == 8);
    assert(s.size == 8);
    assert(s.min_chain >= 1);
    assert(s.max_chain <= 8);
    assert(s.empty_buckets <= 7);
    hash_free(&h);
}

/* ============================================================
 * Bad args
 * ============================================================ */

static void test_bad_args(void)
{
    Hash h;
    HashStats s;

    assert(hash_init(&h, 8) == 0);
    assert(hash_stats(NULL, &s) == -1);
    assert(hash_stats(&h, NULL) == -1);
    hash_free(&h);
}

/* ============================================================
 * main
 * ============================================================ */

int main(void)
{
    printf(">>> test_empty\n");              test_empty();
    printf(">>> test_all_in_one_bucket\n");  test_all_in_one_bucket();
    printf(">>> test_spread\n");             test_spread();
    printf(">>> test_bad_args\n");           test_bad_args();
    printf("all hash_algo tests passed\n");
    return 0;
}
