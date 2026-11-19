/* ============================================================
 * tree_algo.hpp
 * Algorithms over the BST.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#ifndef ALGO_TREE_ALGO_HPP
#define ALGO_TREE_ALGO_HPP

#include "algo/tree.hpp"

/* ============================================================
 * Validate
 * ============================================================ */

/* BST invariant check. 1 = valid, 0 = broken.
 * Duplicates are not allowed: validate returns 0 if any key
 * repeats, which matches tree_insert's duplicate rejection. */
int tree_validate(const Tree* t);

/* ============================================================
 * Min / max
 * ============================================================ */

/* Returns 1 and writes *out on success,
 * 0 if the tree is empty or out is NULL. */
int tree_min(const Tree* t, int* out);
int tree_max(const Tree* t, int* out);

/* ============================================================
 * Array <-> tree
 * ============================================================ */

/* Fill buf (cap >= t->size) with in-order keys.
 * Returns number written (== t->size), or -1 on bad args. */
int tree_to_array_inorder(const Tree* t, int* buf, int cap);

/* Build a perfectly balanced tree from a sorted array.
 * t must be init'd and empty. 0 on OK, -1 on bad args. */
int tree_from_sorted_array(Tree* t, const int* keys, int n);

/* ============================================================
 * Balance
 * ============================================================ */

/* Rebuild t in place, balanced, from its own keys.
 * 0 on OK, -1 on bad args / alloc error. */
int tree_balance(Tree* t);

/* ============================================================
 * BFS
 * ============================================================ */

/* BFS visit, level order. cb(key, user) per node.
 * Returns number of nodes visited. */
typedef void (*TreeVisitFn)(int key, void* user);
int tree_bfs(const Tree* t, TreeVisitFn cb, void* user);

#endif /* ALGO_TREE_ALGO_HPP */
