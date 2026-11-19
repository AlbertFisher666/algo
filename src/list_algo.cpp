/* ============================================================
 * list_algo.cpp
 * Algorithms over the DLL.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/list_algo.hpp"

#include <stdio.h>
#include <stddef.h>

/* ============================================================
 * Merge sort helpers
 * ============================================================ */

/* Merge two sorted chains by next. Stable: <= keeps left
 * before right on equal keys. Rebuilds prev, writes the last
 * node to *tail. */
static ListNode* list_merge_sorted(ListNode* left,
                                   ListNode* right,
                                   ListNode** tail)
{
    ListNode* head;
    ListNode* last;

    head = NULL;
    last = NULL;

    while (left != NULL && right != NULL) {
        ListNode* node;

        if (left->key <= right->key) {
            node = left;
            left = left->next;
        } else {
            node = right;
            right = right->next;
        }

        node->prev = last;
        if (last != NULL)
            last->next = node;
        else
            head = node;
        last = node;
    }

    while (left != NULL) {
        ListNode* node;

        node = left;
        left = left->next;
        node->prev = last;
        if (last != NULL)
            last->next = node;
        else
            head = node;
        last = node;
    }

    while (right != NULL) {
        ListNode* node;

        node = right;
        right = right->next;
        node->prev = last;
        if (last != NULL)
            last->next = node;
        else
            head = node;
        last = node;
    }

    if (last != NULL)
        last->next = NULL;

    *tail = last;
    return head;
}

/* Recursive merge sort over the next chain. Splits at the
 * lower middle, recurses, merges. Writes the last node of
 * the sorted chain to *tail. */
static ListNode* list_sort_chain(ListNode* head, ListNode** tail)
{
    ListNode* slow;
    ListNode* fast;
    ListNode* right;
    ListNode* left_tail;
    ListNode* right_tail;

    if (head == NULL) {
        *tail = NULL;
        return NULL;
    }

    if (head->next == NULL) {
        head->prev = NULL;
        *tail = head;
        return head;
    }

    slow = head;
    fast = head->next;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }

    right = slow->next;
    slow->next = NULL;
    if (right != NULL)
        right->prev = NULL;

    head->prev = NULL;

    head = list_sort_chain(head, &left_tail);
    right = list_sort_chain(right, &right_tail);

    return list_merge_sorted(head, right, tail);
}

/* ============================================================
 * Reverse
 * ============================================================ */

int list_reverse(List* l)
{
    ListNode* node;
    ListNode* old_head;

    if (l == NULL)
        return -1;

    old_head = l->head;
    node = l->head;

    while (node != NULL) {
        ListNode* next;

        next = node->next;
        node->next = node->prev;
        node->prev = next;
        node = next;
    }

    l->head = l->tail;
    l->tail = old_head;

    return 0;
}

/* ============================================================
 * Middle
 * ============================================================ */

ListNode* list_middle(List* l)
{
    ListNode* slow;
    ListNode* fast;

    if (l == NULL || l->head == NULL)
        return NULL;

    slow = l->head;
    fast = l->head->next;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }

    return slow;
}

/* ============================================================
 * Sort
 * ============================================================ */

int list_sort(List* l)
{
    if (l == NULL)
        return -1;

    l->head = list_sort_chain(l->head, &l->tail);
    return 0;
}
