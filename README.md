# algo

A small algorithm library. One file per structure.
No frameworks, no dependencies, 80s C-flavoured C++.

**Status:** v1.0.2 - experiment, not a product. Private repo:
https://github.com/AlbertFisher666/algo

## Requirements

- g++ with C++17
- `make`
- `bash` (for golden tests)

## Build

    make test

Runs unit tests and golden REPL tests. Expected output ends with:

    all unit tests passed
    all golden tests passed

To run only the REPL golden tests:

    make test-repl

To build the REPL:

    make repl

## Layout

    include/algo/tree.hpp            BST - contract
    include/algo/list.hpp            DLL - contract
    include/algo/hash.hpp            hash - contract
    include/algo/tree_algo.hpp       BST algorithms - contract
    include/algo/list_algo.hpp       DLL algorithms - contract
    include/algo/hash_algo.hpp       hash introspection - contract
    src/tree.cpp                     BST - implementation
    src/list.cpp                     DLL - implementation
    src/hash.cpp                     hash - implementation
    src/tree_algo.cpp                BST algorithms - implementation
    src/list_algo.cpp                DLL algorithms - implementation
    src/hash_algo.cpp                hash introspection - implementation
    src/repl.cpp                     interactive REPL + batch modes
    tests/tree_test.cpp              BST - tests
    tests/list_test.cpp              DLL - tests
    tests/hash_test.cpp              hash - tests
    tests/tree_algo_test.cpp         BST algorithms - tests
    tests/tree_algo_bounds_test.cpp  BST algorithm integer-boundary tests
    tests/list_algo_test.cpp         DLL algorithms - tests
    tests/hash_algo_test.cpp         hash introspection - tests
    tests/run_golden.sh              REPL golden tests - runner
    tests/repl_golden/               REPL golden tests - 10 cases (.in/.out)
    Makefile                         build rules

## Build the REPL

    make repl

Then:

    ./build/repl

Or directly, without make:

    g++ -std=c++17 -O2 -Wall -Wextra -I include \
        src/repl.cpp src/tree.cpp src/list.cpp src/hash.cpp \
        src/tree_algo.cpp src/list_algo.cpp src/hash_algo.cpp \
        -o build/repl

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

### tree_algo.hpp

    int tree_validate(const Tree* t);
    int tree_min(const Tree* t, int* out);
    int tree_max(const Tree* t, int* out);
    int tree_to_array_inorder(const Tree* t, int* buf, int cap);
    int tree_from_sorted_array(Tree* t, const int* keys, int n);
    int tree_balance(Tree* t);

    typedef void (*TreeVisitFn)(int key, void* user);
    int tree_bfs(const Tree* t, TreeVisitFn cb, void* user);

### list_algo.hpp

    int list_reverse(List* l);
    ListNode* list_middle(List* l);
    int list_sort(List* l);

### hash_algo.hpp

    struct HashStats {
        int capacity;
        int size;
        int min_chain;     /* 0 if table is empty */
        int max_chain;
        int empty_buckets;
        double load_factor; /* size / capacity */
    };

    int hash_stats(const Hash* h, HashStats* out);

## REPL

`src/repl.cpp` is an interactive front-end to the three structures.
Exactly one structure is live at a time; `teardown()` is the only
place that destroys it. Same style: no iostream, no std::string,
fixed buffers, `printf`.

Interactive commands:

    tree                     create a BST
    list                     create a doubly linked list
    hash [cap]               create a hash table (cap default 16)
    insert <k>               insert key (all structures)
    find <k>                 find key (all structures)
    remove <k>               remove key (all structures)
    push_back <k>            append to list
    push_front <k>           prepend to list
    print                    print all keys, one per line
    draw | schema | ascii    ASCII diagram of current structure
    size                     current element count
    height                   tree height
    validate                 tree: BST invariant check
    min | max                tree: min / max key
    balance                  tree: rebuild balanced
    reverse                  list: reverse in place
    sort                     list: stable merge sort
    stats                    hash: chain statistics
    save <file.repl>         write current structure to a .repl file
    load <file.repl>         read a .repl file into a new structure
    free                     destroy current structure
    help                     this message
    quit | exit              leave

### Batch modes

    ./build/repl --exec "tree; insert 5; insert 3; draw"
    ./build/repl --script tests/repl_golden/case01.in
    ./build/repl --exec "tree; insert abc"      # rc=1
    ./build/repl --help

- `--exec "<commands>"` - split by `;`, execute each; stop on first error.
- `--script <file>` - read file line by line, execute each; stop on first error.
- `--keep-going` - do not stop on errors (both batch modes).
- `--help` - short usage.
- No arguments - interactive mode (prompt, greeting, `quit`/`exit`).

Batch modes do not print `> ` or the greeting. Exit code: `0` on success,
`1` on command error. `quit`/`exit` inside `--exec`/`--script` stops the
batch and returns `0`.

### Diagrams

`draw` prints the live structure as ASCII:

- **tree** - indented BST with `|--` / `` `-- `` branches, followed by
  `size=` and `height=`.
- **list** - `head [k] <-> [k] <-> ... tail`, collapsed with
  `... (N more) ...` when larger than `LIST_DRAW_MAX` (10), followed
  by `size=`.
- **hash** - one line per bucket, `[ i] k -> k -> ...` or `(empty)`,
  followed by `load factor: d.dd`.

All draw routines take `const` pointers and never mutate.

### Save / Load (`.repl`)

Plain text, one int per line. Lines starting with `#` are comments
and are skipped on load; leading and trailing whitespace is trimmed.

    # algo repl save
    # structure: tree
    tree
    5
    3
    8

The first non-comment non-blank line is the tag: `tree`, `list`,
or `hash <capacity>`. Every following data line is one integer key.
`load` builds into temporaries and only swaps in the new structure
on full success, so a broken file never loses live data. Duplicate
keys in the file are warned about and skipped. A missing `.repl`
extension only warns.

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
- `tree_algo`: read-only over the BST. `validate` uses open bounds
  just outside `int` range and strict `<`/`>`, so duplicates are
  rejected (matches `tree_insert`). `to_array_inorder` and
  `from_sorted_array` are inverses for sorted input. `balance` is
  in-order -> array -> `tree_free` -> `from_sorted_array`.
  `bfs` uses an array-backed queue `const TreeNode*[size]`,
  head/tail indices, no `std::queue`.
- `list_algo`: `reverse` is in-place, swaps `prev`/`next` per node,
  then swaps `head`/`tail`. `middle` is two-pointer; for even size
  returns the lower middle (`fast = head->next`). `sort` is stable
  merge sort over `next`, then rebuilds `prev`, `tail`, `size`.
- `hash_algo`: one pass over buckets. `min_chain` counts non-empty
  buckets only; for an empty table it is 0, same as `max_chain`.
  `load_factor = size / capacity` as `double`.
- `repl`: single global structure, `Kind` tag, `teardown()` as the
  one destruction point. Dispatch lives in `execute_line(line)`
  (returns 0 OK, 1 error, 2 quit); interactive mode and batch modes
  (`--exec`, `--script`) share the same dispatcher. Integer parsing
  is `strtol` with full-string and range checks (`parse_whole_int`).
  Save writes in-order for the tree, head-to-tail for the list,
  bucket order for the hash.

## Tests

    make test

Runs unit tests and golden REPL tests.

Unit tests (assert-based, no framework):

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

`tree_algo_test.cpp` covers:

- validate: empty, good, broken (wrong child on the left)
- min / max: empty, extremes
- to_array_inorder: sorted output, length == size
- from_sorted_array: perfectly balanced, validate
- balance: degenerate 1..7 -> height 3, validate
- bfs: level-order via callback

`tree_algo_bounds_test.cpp` covers:

- `INT_MIN` / `INT_MAX`: build from a sorted array, validate,
min / max, and in-order output.

`list_algo_test.cpp` covers:

- reverse: empty, one, even, odd - checks head/tail/prev/next
- middle: empty, one, odd, even (lower middle)
- sort: empty, one, reverse input, sorted input, duplicates
- prev-chain: `check_forward` walks head->tail and verifies
  `cur->prev == prev` and `l->tail == last`

`hash_algo_test.cpp` covers:

- empty: capacity 8, all counters zero, `empty_buckets == 8`
- all in one bucket: capacity 1, three keys, `min == max == 3`
- spread: capacity 8, eight keys, fields consistent
- bad args: `NULL` h / `NULL` out -> -1

Golden REPL tests:

    make test-repl

`tests/repl_golden/` has 10 cases (`case01.in` … `case10.in` with
matching `.out`). Each `.in` is fed via `--script`, output is diffed
against `.out`. `run_golden.sh` normalizes prompts (`^> `) and file
paths (`<FILE>`); `LC_ALL=C` is set so `strerror` is stable.
Cases: tree (insert/draw/validate/min/max/height/size), balance,
list (push/reverse/sort), hash (stats/draw/remove), save/load
tree/list/hash, errors, duplicates, free + recreate.

## CI

`.github/workflows/ci.yml` runs `make` and `make test` on every push
and pull request. `make test` includes unit tests and golden REPL
tests. Same skeleton as `cpvm-cpp`: checkout, compiler version,
build, test.

## License

MIT. See `LICENSE`.

Copyright (c) 2026 Albert Ghosting Frangle (Albert Fisher)
