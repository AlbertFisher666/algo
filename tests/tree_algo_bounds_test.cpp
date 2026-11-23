#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "algo/tree.hpp"
#include "algo/tree_algo.hpp"

static void test_int_boundaries(void) {
    Tree t;
    tree_init(&t);

    int keys[4];
    keys[0] = INT_MIN;
    keys[1] = -1;
    keys[2] = 0;
    keys[3] = INT_MAX;

    assert(tree_from_sorted_array(&t, keys, 4) == 0);
    assert(tree_validate(&t) == 1);

    int value = 0;

    assert(tree_min(&t, &value) == 1);
    assert(value == INT_MIN);

    assert(tree_max(&t, &value) == 1);
    assert(value == INT_MAX);

    int out[4];
    assert(tree_to_array_inorder(&t, out, 4) == 4);

    for (int i = 0; i < 4; i++) {
        assert(out[i] == keys[i]);
    }

    tree_free(&t);
}

int main(void) {
    printf(">>> test_int_boundaries\n");
    test_int_boundaries();

    printf("all tree algo bounds tests passed\n");
    return 0;
}
