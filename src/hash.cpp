/* ============================================================
 * hash.cpp
 * Chained hash table. Integer keys. FNV-1a hash.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/hash.hpp"

#include <stdio.h>
#include <stddef.h>

static unsigned int hash_fn(int key, int cap) {
    unsigned int h = 2166136261u;             /* FNV-1a offset */
    unsigned char* p = (unsigned char*)&key;
    for (int i = 0; i < (int)sizeof(int); i++) {
        h ^= p[i];
        h *= 16777619u;
    }
    return h % (unsigned int)cap;
}

/* ============================================================
 * Lifecycle
 * ============================================================ */

int hash_init(Hash* h, int capacity) {
    if (capacity <= 0) return -1;
    h->buckets = new HashNode*[capacity];
    for (int i = 0; i < capacity; i++) h->buckets[i] = NULL;
    h->capacity = capacity;
    h->size = 0;
    return 0;
}

void hash_free(Hash* h) {
    if (h->buckets == NULL) return;
    for (int i = 0; i < h->capacity; i++) {
        HashNode* cur = h->buckets[i];
        while (cur != NULL) {
            HashNode* next = cur->next;
            delete cur;
            cur = next;
        }
    }
    delete[] h->buckets;
    h->buckets = NULL;
    h->capacity = 0;
    h->size = 0;
}

/* ============================================================
 * Insert / Find / Remove
 * ============================================================ */

int hash_insert(Hash* h, int key) {
    unsigned int idx = hash_fn(key, h->capacity);
    for (HashNode* cur = h->buckets[idx]; cur != NULL; cur = cur->next) {
        if (cur->key == key) return -1;  /* duplicate */
    }
    HashNode* n = new HashNode;
    n->key  = key;
    n->next = h->buckets[idx];
    h->buckets[idx] = n;
    h->size++;
    return 0;
}

int hash_find(const Hash* h, int key) {
    unsigned int idx = hash_fn(key, h->capacity);
    for (HashNode* cur = h->buckets[idx]; cur != NULL; cur = cur->next) {
        if (cur->key == key) return 1;
    }
    return 0;
}

int hash_remove(Hash* h, int key) {
    unsigned int idx = hash_fn(key, h->capacity);
    HashNode** slot = &h->buckets[idx];
    while (*slot != NULL && (*slot)->key != key) {
        slot = &(*slot)->next;
    }
    if (*slot == NULL) return -1;
    HashNode* victim = *slot;
    *slot = victim->next;
    delete victim;
    h->size--;
    return 0;
}

/* ============================================================
 * Print
 * ============================================================ */

void hash_print(const Hash* h) {
    for (int i = 0; i < h->capacity; i++) {
        for (HashNode* cur = h->buckets[i]; cur != NULL; cur = cur->next) {
            printf("%d\n", cur->key);
        }
    }
}
