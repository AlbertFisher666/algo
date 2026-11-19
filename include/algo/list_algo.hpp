/* ============================================================
 * list_algo.hpp
 * Algorithms over the doubly linked list.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#ifndef ALGO_LIST_ALGO_HPP
#define ALGO_LIST_ALGO_HPP

#include "algo/list.hpp"

/* ============================================================
 * Reverse
 * ============================================================ */

/* Reverse in place. 0 on OK, -1 on NULL. */
int list_reverse(List* l);

/* ============================================================
 * Middle
 * ============================================================ */

/* Middle node (two-pointer). NULL on empty list.
 * For even size, returns the lower middle. */
ListNode* list_middle(List* l);

/* ============================================================
 * Sort
 * ============================================================ */

/* Stable merge sort by key, in place. 0 on OK, -1 on NULL. */
int list_sort(List* l);

#endif /* ALGO_LIST_ALGO_HPP */
