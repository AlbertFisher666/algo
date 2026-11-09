#ifndef ALGO_HASH_HPP
#define ALGO_HASH_HPP

/* hash.hpp - chained hash table contract. 80s C-flavoured C++. */

struct HashNode {
    int key;
    HashNode* next;
};

struct Hash {
    HashNode** buckets;
    int capacity;
    int size;
};

/* init: capacity must be > 0; 0 on OK, -1 on error */
int  hash_init(Hash* h, int capacity);

/* free all buckets and nodes */
void hash_free(Hash* h);

/* insert key; 0 on OK, -1 on duplicate or error */
int  hash_insert(Hash* h, int key);

/* find key; 1 if present, 0 otherwise */
int  hash_find(const Hash* h, int key);

/* remove key; 0 on OK, -1 if not found */
int  hash_remove(Hash* h, int key);

/* print all keys, one per line */
void hash_print(const Hash* h);

#endif /* ALGO_HASH_HPP */
