/* ============================================================
 * hash_algo.hpp
 * Introspection over the chained hash table.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#ifndef ALGO_HASH_ALGO_HPP
#define ALGO_HASH_ALGO_HPP

#include "algo/hash.hpp"

/* ============================================================
 * Stats
 * ============================================================ */

struct HashStats {
    int    capacity;
    int    size;
    int    min_chain;     /* 0 if table is empty */
    int    max_chain;
    int    empty_buckets;
    double load_factor;   /* size / capacity */
};

/* Fill *out. 0 on OK, -1 on bad args.
 * min_chain is computed over non-empty buckets only and is 0
 * for an empty table. */
int hash_stats(const Hash* h, HashStats* out);

#endif /* ALGO_HASH_ALGO_HPP */
