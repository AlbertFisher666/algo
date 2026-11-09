#ifndef ALGO_LIST_HPP
#define ALGO_LIST_HPP

/* list.hpp - doubly linked list contract. 80s C-flavoured C++. */

struct ListNode {
    int key;
    ListNode* prev;
    ListNode* next;
};

struct List {
    ListNode* head;
    ListNode* tail;
    int size;
};

/* init: head=tail=NULL, size=0 */
void list_init(List* l);

/* recursive-free is not needed: iterative, no stack overflow */
void list_free(List* l);

/* push to tail; 0 on OK, -1 on error */
int list_push_back(List* l, int key);

/* push to head; 0 on OK, -1  on error */
int list_push_front(List* l, int key);

/* find first node with key; returns pointer or NULL */
ListNode* list_find(List* l, int key);

/* remove first node with key; 0 on OK, -1 if not found */
int list_remove(List* l, int key);

/* in-order (head -> tail) print */
void list_print(const List* l);

#endif /* ALGO_LIST_HPP */
