/* ============================================================
 * list.cpp
 * Doubly linked list. Integer keys. Iterative.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/list.hpp"

#include <stdio.h>

/* ============================================================
 * Lifecycle
 * ============================================================ */

void list_init(List* l) {
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
}

void list_free(List* l) {
    ListNode* cur = l->head;
    while (cur != NULL) {
        ListNode* next = cur->next;
        delete cur;
        cur = next;
    }
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
}

/* ============================================================
 * Push
 * ============================================================ */

int list_push_back(List* l, int key) {
    ListNode* n = new ListNode;
    n->key  = key;
    n->prev = l->tail;
    n->next = NULL;
    if (l->tail != NULL) {
        l->tail->next = n;
    } else {
        l->head = n;
    }
    l->tail = n;
    l->size++;
    return 0;
}

int list_push_front(List* l, int key) {
    ListNode* n = new ListNode;
    n->key  = key;
    n->prev = NULL;
    n->next = l->head;
    if (l->head != NULL) {
        l->head->prev = n;
    } else {
        l->tail = n;
    }
    l->head = n;
    l->size++;
    return 0;
}

/* ============================================================
 * Find / Remove
 * ============================================================ */

ListNode* list_find(List* l, int key) {
    ListNode* cur = l->head;
    while (cur != NULL) {
        if (cur->key == key) return cur;
        cur = cur->next;
    }
    return NULL;
}

int list_remove(List* l, int key) {
    ListNode* cur = l->head;
    while (cur != NULL && cur->key != key) {
        cur = cur->next;
    }
    if (cur == NULL) return -1;

    if (cur->prev != NULL) cur->prev->next = cur->next;
    else                   l->head = cur->next;

    if (cur->next != NULL) cur->next->prev = cur->prev;
    else                   l->tail = cur->prev;

    delete cur;
    l->size--;
    return 0;
}

/* ============================================================
 * Print
 * ============================================================ */

void list_print(const List* l) {
    const ListNode* cur = l->head;
    while (cur != NULL) {
        printf("%d\n", cur->key);
        cur = cur->next;
    }
}
