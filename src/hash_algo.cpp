/* ============================================================
 * hash_algo.cpp
 * Introspection over the chained hash.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "algo/hash_algo.hpp"

#include <stdio.h>
#include <stddef.h>

/* ============================================================
 * Stats
 * ============================================================ */

/* One pass over buckets. min_chain counts non-empty buckets
 * only; for an empty table it is 0, same as max_chain. */
int hash_stats(const Hash* h, HashStats* out)
{
    int min_chain;
    int max_chain;
    int empty;
    int i;

    if (h == NULL || out == NULL)
        return -1;
    if (h->capacity <= 0)
        return -1;

    min_chain = -1;
    max_chain = 0;
    empty = 0;

    for (i = 0; i < h->capacity; i++) {
        HashNode* cur;
        int len;

        len = 0;
        for (cur = h->buckets[i]; cur != NULL; cur = cur->next)
            len++;

        if (len == 0)
            empty++;
        else if (min_chain < 0 || len < min_chain)
            min_chain = len;

        if (len > max_chain)
            max_chain = len;
    }

    out->capacity      = h->capacity;
    out->size          = h->size;
    out->min_chain     = (min_chain < 0) ? 0 : min_chain;
    out->max_chain     = max_chain;
    out->empty_buckets = empty;
    out->load_factor   = (double)h->size / (double)h->capacity;
    return 0;
}
