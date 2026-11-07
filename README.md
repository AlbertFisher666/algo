# algo

A small algorithm library. One file per structure.
No frameworks, no dependencies, 80s C-flavoured C++.

**Status:** experiment. Not a product. No GitHub (yet).

## Requirements

- g++ with C++17
- `make`

## Build

    make test

Expected output ends with:

    all tests passed

## Layout

    include/algo/tree.hpp   BST — contract
    src/tree.cpp            BST — implementation
    tests/tree_test.cpp     BST — tests (assert-based)
    Makefile                build rules

## API

### tree.hpp

    struct TreeNode { int key; TreeNode* left; TreeNode* right; };
    struct Tree     { TreeNode* root; int size; };

    void tree_init(Tree* t);
    void tree_free(Tree* t);
    int  tree_insert(Tree* t, int key);
    int  tree_find(const Tree* t, int key);
    int  tree_remove(Tree* t, int key);
    int  tree_height(const Tree* t);
    void tree_print(const Tree* t);

## Design notes

- `tree_insert` is iterative, via `TreeNode**`. No recursion.
  No stack overflow on deep trees.
- `tree_free` is recursive, post-order. Children first, then node.
- `tree_find` takes `const Tree*`. It does not mutate.
- `tree_remove` has three cases: leaf, one child, two children.
  Two-children case replaces the key with the min of the right
  subtree, then detaches that min.
- `tree_height` is recursive. Height of empty tree is 0.
- `tree_print` is in-order. Output is sorted.

## Tests

    make test

`tree_test.cpp` covers:

- insert: empty, basic, duplicate
- find: present, absent
- height: empty, one, two, three levels
- remove: leaf, one child, two children
- print: in-order output

No framework. `assert` from `<assert.h>`.

## License

Personal experiment. Not for redistribution.

Copyright (c) 2026 Albert Ghosting Frangle (Albert Fisher)
