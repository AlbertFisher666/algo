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
    include/algo/list.hpp   DLL — contract
    include/algo/hash.hpp   hash — contract
    src/tree.cpp            BST — implementation
    src/list.cpp            DLL — implementation
    src/hash.cpp            hash — implementation
    tests/tree_test.cpp     BST — tests
    tests/list_test.cpp     DLL — tests
    tests/hash_test.cpp     hash — tests
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

### list.hpp

    struct ListNode { int key; ListNode* prev; ListNode* next; };
    struct List     { ListNode* head; ListNode* tail; int size; };

    void list_init(List* l);
    void list_free(List* l);
    int  list_push_back(List* l, int key);
    int  list_push_front(List* l, int key);
    ListNode* list_find(List* l, int key);
    int  list_remove(List* l, int key);
    void list_print(const List* l);

### hash.hpp

    struct HashNode { int key; HashNode* next; };
    struct Hash     { HashNode** buckets; int capacity; int size; };

    int  hash_init(Hash* h, int capacity);
    void hash_free(Hash* h);
    int  hash_insert(Hash* h, int key);
    int  hash_find(const Hash* h, int key);
    int  hash_remove(Hash* h, int key);
    void hash_print(const Hash* h);

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
- `list`: doubly linked. Iterative free. Push back/front.
  Remove handles head, tail, middle.
- `hash`: chained. FNV-1a over the key bytes. No resize.
  Capacity is fixed at `hash_init`.

## Tests

    make test

`tree_test.cpp` covers:

- insert: empty, basic, duplicate
- find: present, absent
- height: empty, one, two, three levels
- remove: leaf, one child, two children
- print: in-order output

`list_test.cpp` covers:

- init: empty
- push: back, front
- find: present, absent
- remove: head, tail, middle

`hash_test.cpp` covers:

- init: bad capacity
- insert: duplicate, collision
- find: present, absent
- remove: present, absent

No framework. `assert` from `<assert.h>`.

## License

Personal experiment. Not for redistribution.

Copyright (c) 2026 Albert Ghosting Frangle (Albert Fisher)
