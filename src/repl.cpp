/* ============================================================
 * repl.cpp
 * Interactive REPL for the algo library (tree / list / hash),
 * with ASCII diagrams, .repl save/load, and batch modes.
 *
 * Build:
 *   make repl
 * or:
 *   g++ -std=c++17 -O2 -Wall -Wextra -I include \
 *       src/repl.cpp src/tree.cpp src/list.cpp src/hash.cpp \
 *       src/tree_algo.cpp src/list_algo.cpp src/hash_algo.cpp \
 *       -o build/repl
 *
 * Style: 80s C-flavoured C++. No iostream, no std::string.
 * ============================================================ */

#include "algo/hash_algo.hpp"
#include "algo/hash.hpp"
#include "algo/list_algo.hpp"
#include "algo/list.hpp"
#include "algo/tree_algo.hpp"
#include "algo/tree.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* ============================================================
 * 1. Global state
 *
 * Exactly one structure is live at a time. g_kind says which.
 * teardown() is the only place that destroys it.
 * ============================================================ */

enum Kind { KIND_NONE, KIND_TREE, KIND_LIST, KIND_HASH };

static Kind g_kind = KIND_NONE;
static Tree g_tree;
static List g_list;
static Hash g_hash;

static const char* kind_name(Kind k) {
    switch (k) {
    case KIND_TREE: return "tree";
    case KIND_LIST: return "list";
    case KIND_HASH: return "hash";
    default:        return "none";
    }
}

static void teardown(void) {
    if (g_kind == KIND_TREE) tree_free(&g_tree);
    if (g_kind == KIND_LIST) list_free(&g_list);
    if (g_kind == KIND_HASH) hash_free(&g_hash);
    g_kind = KIND_NONE;
}

/* ============================================================
 * 2. ASCII diagrams (read-only, const pointers)
 * ============================================================ */

static void tree_draw_node(const TreeNode* n,
                           const char* prefix,
                           int is_last,
                           int is_root) {
    if (n == NULL) return;

    if (is_root) {
        printf("%d\n", n->key);
    } else {
        printf("%s%s%d\n", prefix, is_last ? "`-- " : "|-- ", n->key);
    }

    char child_prefix[256];
    if (is_root) {
        snprintf(child_prefix, sizeof child_prefix, "%s", prefix);
    } else {
        snprintf(child_prefix, sizeof child_prefix, "%s%s",
                 prefix, is_last ? "    " : "|   ");
    }

    int has_left  = (n->left  != NULL);
    int has_right = (n->right != NULL);

    if (has_left)  tree_draw_node(n->left,  child_prefix, !has_right, 0);
    if (has_right) tree_draw_node(n->right, child_prefix, 1, 0);
}

static void draw_tree(const Tree* t) {
    if (t->root == NULL) {
        printf("(empty tree)\n");
        return;
    }
    tree_draw_node(t->root, "", 1, 1);
    printf("size=%d height=%d\n", t->size, tree_height(t));
}

#define LIST_DRAW_MAX 10

static void draw_list(const List* l) {
    if (l->head == NULL) {
        printf("(empty list)\n");
        return;
    }

    printf("head ");
    const ListNode* cur = l->head;
    int shown = 0;
    int jumped = 0;
    int total = l->size;

    while (cur != NULL) {
        if (total > LIST_DRAW_MAX && !jumped &&
            shown == LIST_DRAW_MAX / 2) {
            jumped = 1;
            int skipped = total - LIST_DRAW_MAX;
            printf("<-> ... (%d more) ... <-> ", skipped);
            int skip = total - LIST_DRAW_MAX / 2;
            cur = l->head;
            for (int i = 0; i < skip; i++) cur = cur->next;
            shown = LIST_DRAW_MAX / 2;
        }
        printf("[%d]", cur->key);
        shown++;
        cur = cur->next;
        if (cur != NULL) printf(" <-> ");
    }
    printf(" tail\n");
    printf("size=%d\n", l->size);
}

static void draw_hash(const Hash* h) {
    printf("hash (capacity=%d, size=%d)\n", h->capacity, h->size);
    for (int i = 0; i < h->capacity; i++) {
        printf("[%2d] ", i);
        HashNode* cur = h->buckets[i];
        if (cur == NULL) {
            printf("(empty)\n");
            continue;
        }
        while (cur != NULL) {
            printf("%d", cur->key);
            if (cur->next != NULL) printf(" -> ");
            cur = cur->next;
        }
        printf("\n");
    }
    if (h->capacity > 0) {
        int lf100 = (h->size * 100) / h->capacity;
        printf("load factor: %d.%02d\n", lf100 / 100, lf100 % 100);
    }
}

/* ============================================================
 * 3. Commands
 * ============================================================ */

static int need(Kind k, const char* what) {
    if (g_kind != k) {
        printf("error: %s requires a %s (current: %s)\n",
               what, kind_name(k), kind_name(g_kind));
        return 0;
    }
    return 1;
}

static void cmd_tree(void) {
    teardown();
    tree_init(&g_tree);
    g_kind = KIND_TREE;
    printf("tree created\n");
}

static void cmd_list(void) {
    teardown();
    list_init(&g_list);
    g_kind = KIND_LIST;
    printf("list created\n");
}

static void cmd_hash(int cap) {
    teardown();
    if (hash_init(&g_hash, cap) != 0) {
        printf("error: bad capacity %d\n", cap);
        return;
    }
    g_kind = KIND_HASH;
    printf("hash created (capacity %d)\n", cap);
}

static void cmd_insert(int key) {
    int rc = -1;
    switch (g_kind) {
    case KIND_TREE: rc = tree_insert(&g_tree, key); break;
    case KIND_LIST: rc = list_push_back(&g_list, key); break;
    case KIND_HASH: rc = hash_insert(&g_hash, key); break;
    default: printf("error: no structure\n"); return;
    }
    if (rc != 0) printf("error: insert %d failed (duplicate?)\n", key);
    else         printf("ok\n");
}

static void cmd_find(int key) {
    int found = 0;
    switch (g_kind) {
    case KIND_TREE: found = tree_find(&g_tree, key); break;
    case KIND_LIST: found = (list_find(&g_list, key) != NULL); break;
    case KIND_HASH: found = hash_find(&g_hash, key); break;
    default: printf("error: no structure\n"); return;
    }
    printf(found ? "found\n" : "not found\n");
}

static void cmd_remove(int key) {
    int rc = -1;
    switch (g_kind) {
    case KIND_TREE: rc = tree_remove(&g_tree, key); break;
    case KIND_LIST: rc = list_remove(&g_list, key); break;
    case KIND_HASH: rc = hash_remove(&g_hash, key); break;
    default: printf("error: no structure\n"); return;
    }
    if (rc != 0) printf("error: %d not found\n", key);
    else         printf("ok\n");
}

static void cmd_push_back(int key) {
    if (!need(KIND_LIST, "push_back")) return;
    list_push_back(&g_list, key);
    printf("ok\n");
}

static void cmd_push_front(int key) {
    if (!need(KIND_LIST, "push_front")) return;
    list_push_front(&g_list, key);
    printf("ok\n");
}

static void cmd_print(void) {
    switch (g_kind) {
    case KIND_TREE: tree_print(&g_tree); break;
    case KIND_LIST: list_print(&g_list); break;
    case KIND_HASH: hash_print(&g_hash); break;
    default: printf("error: no structure\n"); return;
    }
}

static void cmd_size(void) {
    int s = -1;
    switch (g_kind) {
    case KIND_TREE: s = g_tree.size; break;
    case KIND_LIST: s = g_list.size; break;
    case KIND_HASH: s = g_hash.size; break;
    default: printf("error: no structure\n"); return;
    }
    printf("%d\n", s);
}

static void cmd_height(void) {
    if (!need(KIND_TREE, "height")) return;
    printf("%d\n", tree_height(&g_tree));
}

static void cmd_validate(void) {
    if (!need(KIND_TREE, "validate")) return;
    printf(tree_validate(&g_tree) ? "valid\n" : "BROKEN\n");
}

static void cmd_min(void) {
    if (!need(KIND_TREE, "min")) return;
    int v;
    if (tree_min(&g_tree, &v)) printf("%d\n", v);
    else                       printf("(empty)\n");
}

static void cmd_max(void) {
    if (!need(KIND_TREE, "max")) return;
    int v;
    if (tree_max(&g_tree, &v)) printf("%d\n", v);
    else                       printf("(empty)\n");
}

static void cmd_balance(void) {
    if (!need(KIND_TREE, "balance")) return;
    if (tree_balance(&g_tree) == 0) printf("ok\n");
    else                            printf("error: balance failed\n");
}

static void cmd_reverse(void) {
    if (!need(KIND_LIST, "reverse")) return;
    list_reverse(&g_list);
    printf("ok\n");
}

static void cmd_sort(void) {
    if (!need(KIND_LIST, "sort")) return;
    list_sort(&g_list);
    printf("ok\n");
}

static void cmd_stats(void) {
    if (!need(KIND_HASH, "stats")) return;
    HashStats s;
    if (hash_stats(&g_hash, &s) != 0) { printf("error: stats failed\n"); return; }
    printf("capacity=%d size=%d\n", s.capacity, s.size);
    printf("min_chain=%d max_chain=%d empty_buckets=%d\n",
           s.min_chain, s.max_chain, s.empty_buckets);
    printf("load_factor=%.2f\n", s.load_factor);
}

static void cmd_draw(void) {
    switch (g_kind) {
    case KIND_TREE: draw_tree(&g_tree); break;
    case KIND_LIST: draw_list(&g_list); break;
    case KIND_HASH: draw_hash(&g_hash); break;
    default: printf("error: no structure\n"); return;
    }
}

static void cmd_free(void) {
    teardown();
    printf("freed\n");
}

static void cmd_help(void) {
    printf(
        "commands:\n"
        "  tree                     create a BST\n"
        "  list                     create a doubly linked list\n"
        "  hash [cap]               create a hash table (cap default 16)\n"
        "  insert <k>               insert key (all structures)\n"
        "  find <k>                 find key (all structures)\n"
        "  remove <k>               remove key (all structures)\n"
        "  push_back <k>            append to list\n"
        "  push_front <k>           prepend to list\n"
        "  print                    print all keys, one per line\n"
        "  draw | schema | ascii    ASCII diagram of current structure\n"
        "  size                     current element count\n"
        "  height                   tree height\n"
        "  validate                 tree: BST invariant check\n"
        "  min | max                tree: min / max key\n"
        "  balance                  tree: rebuild balanced\n"
        "  reverse                  list: reverse in place\n"
        "  sort                     list: stable merge sort\n"
        "  stats                    hash: chain statistics\n"
        "  save <file.repl>         write current structure to a .repl file\n"
        "  load <file.repl>         read a .repl file into a new structure\n"
        "  free                     destroy current structure\n"
        "  help                     this message\n"
        "  quit | exit              leave\n");
}

/* ============================================================
 * 4. Save / Load (.repl format)
 * ============================================================ */

static char* trim(char* buf) {
    char* s = buf;
    while (*s == ' ' || *s == '\t') s++;
    if (s != buf) memmove(buf, s, strlen(s) + 1);
    size_t len = strlen(buf);
    while (len > 0 &&
           (buf[len - 1] == ' ' || buf[len - 1] == '\t' ||
            buf[len - 1] == '\r' || buf[len - 1] == '\n')) {
        buf[--len] = '\0';
    }
    return buf;
}

static int parse_whole_int(const char* s, int* out) {
    if (s == NULL || *s == '\0') return 0;
    errno = 0;
    char* end = NULL;
    long v = strtol(s, &end, 10);
    if (errno == ERANGE) return 0;
    if (*end != '\0') return 0;
    if (v < -2147483647L - 1 || v > 2147483647L) return 0;
    *out = (int)v;
    return 1;
}

static int has_repl_ext(const char* path) {
    size_t n = strlen(path);
    return n >= 5 && strcmp(path + n - 5, ".repl") == 0;
}

static void save_write_keys(FILE* f, const int* keys, int n) {
    for (int i = 0; i < n; i++) fprintf(f, "%d\n", keys[i]);
}

static void save_tree_keys(const TreeNode* n, int* buf, int* pos) {
    if (n == NULL) return;
    save_tree_keys(n->left, buf, pos);
    buf[(*pos)++] = n->key;
    save_tree_keys(n->right, buf, pos);
}

static int save_tree(FILE* f, const Tree* t) {
    if (fprintf(f, "tree\n") < 0) return -1;
    if (t->size == 0) return 0;
    int* buf = new int[t->size];
    int pos = 0;
    save_tree_keys(t->root, buf, &pos);
    save_write_keys(f, buf, t->size);
    delete[] buf;
    return 0;
}

static int save_list(FILE* f, const List* l) {
    if (fprintf(f, "list\n") < 0) return -1;
    for (const ListNode* cur = l->head; cur != NULL; cur = cur->next) {
        if (fprintf(f, "%d\n", cur->key) < 0) return -1;
    }
    return 0;
}

static int save_hash(FILE* f, const Hash* h) {
    if (fprintf(f, "hash %d\n", h->capacity) < 0) return -1;
    for (int i = 0; i < h->capacity; i++) {
        for (HashNode* cur = h->buckets[i]; cur != NULL; cur = cur->next) {
            if (fprintf(f, "%d\n", cur->key) < 0) return -1;
        }
    }
    return 0;
}

static int save_current(const char* path) {
    if (g_kind == KIND_NONE) {
        printf("error: no structure\n");
        return -1;
    }
    if (!has_repl_ext(path)) {
        printf("warning: '%s' does not end with .repl\n", path);
    }
    FILE* f = fopen(path, "w");
    if (f == NULL) {
        printf("error: cannot open '%s' for writing: %s\n",
               path, strerror(errno));
        return -1;
    }

    fprintf(f, "# algo repl save\n");
    fprintf(f, "# structure: %s\n", kind_name(g_kind));

    int rc = 0;
    switch (g_kind) {
    case KIND_TREE: rc = save_tree(f, &g_tree); break;
    case KIND_LIST: rc = save_list(f, &g_list); break;
    case KIND_HASH: rc = save_hash(f, &g_hash); break;
    default: break;
    }
    if (rc != 0) {
        printf("error: write failed for '%s'\n", path);
        fclose(f);
        return -1;
    }
    if (fclose(f) != 0) {
        printf("error: close failed for '%s': %s\n",
               path, strerror(errno));
        return -1;
    }

    int n = g_kind == KIND_TREE ? g_tree.size :
            g_kind == KIND_LIST ? g_list.size : g_hash.size;
    printf("saved %s (%d keys)\n", path, n);
    return 0;
}

static int read_data_line(FILE* f, char* buf, int cap) {
    while (fgets(buf, cap, f) != NULL) {
        trim(buf);
        if (buf[0] == '\0' || buf[0] == '#') continue;
        return 1;
    }
    return 0;
}

static Kind parse_tag(const char* line, int* cap) {
    if (strcmp(line, "tree") == 0) return KIND_TREE;
    if (strcmp(line, "list") == 0) return KIND_LIST;
    if (strncmp(line, "hash", 4) == 0 &&
        (line[4] == '\0' || line[4] == ' ' || line[4] == '\t')) {
        int c = 16;
        const char* rest = line + 4;
        while (*rest == ' ' || *rest == '\t') rest++;
        if (*rest != '\0') {
            if (!parse_whole_int(rest, &c)) return KIND_NONE;
            if (c <= 0) return KIND_NONE;
        }
        *cap = c;
        return KIND_HASH;
    }
    return KIND_NONE;
}

static int load_from(const char* path) {
    if (!has_repl_ext(path)) {
        printf("warning: '%s' does not end with .repl\n", path);
    }
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        printf("error: cannot open '%s': %s\n", path, strerror(errno));
        return -1;
    }

    char line[256];
    if (!read_data_line(f, line, sizeof line)) {
        printf("error: '%s' has no data line\n", path);
        fclose(f);
        return -1;
    }

    int cap = 16;
    Kind k = parse_tag(line, &cap);
    if (k == KIND_NONE) {
        printf("error: '%s': bad structure tag '%s'\n", path, line);
        fclose(f);
        return -1;
    }

    Tree t;  tree_init(&t);
    List l;  list_init(&l);
    Hash h;  int hash_ready = 0;
    int  n = 0;
    int  bad = 0;

    while (read_data_line(f, line, sizeof line)) {
        int key = 0;
        if (!parse_whole_int(line, &key)) {
            printf("error: '%s': bad key line '%s'\n", path, line);
            bad = 1;
            break;
        }
        switch (k) {
        case KIND_TREE:
            if (tree_insert(&t, key) != 0)
                printf("warning: duplicate key %d (skipped)\n", key);
            else n++;
            break;
        case KIND_LIST:
            list_push_back(&l, key);
            n++;
            break;
        case KIND_HASH:
            if (!hash_ready) {
                if (hash_init(&h, cap) != 0) {
                    printf("error: bad capacity %d\n", cap);
                    bad = 1;
                } else hash_ready = 1;
            }
            if (bad) break;
            if (hash_insert(&h, key) != 0)
                printf("warning: duplicate key %d (skipped)\n", key);
            else n++;
            break;
        default: break;
        }
        if (bad) break;
    }
    fclose(f);

    if (bad) {
        tree_free(&t);
        list_free(&l);
        if (hash_ready) hash_free(&h);
        return -1;
    }

    teardown();
    switch (k) {
    case KIND_TREE: g_tree = t; g_kind = KIND_TREE; break;
    case KIND_LIST: g_list = l; g_kind = KIND_LIST; break;
    case KIND_HASH:
        if (!hash_ready && hash_init(&h, cap) != 0) {
            printf("error: bad capacity %d\n", cap);
            return -1;
        }
        g_hash = h; g_kind = KIND_HASH; break;
    default: break;
    }

    printf("loaded %s (%d keys)\n", path, n);
    return 0;
}

/* ============================================================
 * 5. Parsing helpers
 * ============================================================ */

static int next_token(char** p, char* tok, int tokcap, char** rest) {
    char* s = *p;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0') { *rest = s; return 0; }
    int n = 0;
    while (*s != '\0' && *s != ' ' && *s != '\t' && n < tokcap - 1) {
        tok[n++] = *s++;
    }
    tok[n] = '\0';
    while (*s == ' ' || *s == '\t') s++;
    *rest = s;
    *p = s;
    return 1;
}

/* ============================================================
 * 6. execute_line
 *
 * Dispatch one line. Returns:
 *   0 - command executed (OK)
 *   1 - command error (unknown command, bad arg, need() failed)
 *   2 - quit / exit
 *
 * Modifies the line in place (trims \n, next_token moves
 * pointers but does not write past the token).
 * ============================================================ */

static int execute_line(char* line) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }

    char tok[64];
    char* rest = NULL;
    char* p = line;
    if (!next_token(&p, tok, sizeof tok, &rest)) return 0;

    if (strcmp(tok, "quit") == 0 || strcmp(tok, "exit") == 0) return 2;
    if (strcmp(tok, "help") == 0)    { cmd_help(); return 0; }
    if (strcmp(tok, "tree") == 0)    { cmd_tree(); return 0; }
    if (strcmp(tok, "list") == 0)    { cmd_list(); return 0; }
    if (strcmp(tok, "hash") == 0) {
        int cap = 16;
        if (*rest != '\0' && !parse_whole_int(rest, &cap)) {
            printf("error: bad capacity\n"); return 1;
        }
        cmd_hash(cap); return 0;
    }
    if (strcmp(tok, "insert") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: insert needs an integer\n"); return 1; }
        cmd_insert(k); return 0;
    }
    if (strcmp(tok, "find") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: find needs an integer\n"); return 1; }
        cmd_find(k); return 0;
    }
    if (strcmp(tok, "remove") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: remove needs an integer\n"); return 1; }
        cmd_remove(k); return 0;
    }
    if (strcmp(tok, "push_back") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: push_back needs an integer\n"); return 1; }
        cmd_push_back(k); return 0;
    }
    if (strcmp(tok, "push_front") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: push_front needs an integer\n"); return 1; }
        cmd_push_front(k); return 0;
    }
    if (strcmp(tok, "print") == 0) { cmd_print(); return 0; }
    if (strcmp(tok, "draw") == 0 ||
        strcmp(tok, "schema") == 0 ||
        strcmp(tok, "ascii") == 0) { cmd_draw(); return 0; }
    if (strcmp(tok, "size") == 0)   { cmd_size(); return 0; }
    if (strcmp(tok, "height") == 0) { cmd_height(); return 0; }
    if (strcmp(tok, "save") == 0) {
        if (*rest == '\0') { printf("error: save needs a file path\n"); return 1; }
        return save_current(rest) == 0 ? 0 : 1;
    }
    if (strcmp(tok, "load") == 0) {
        if (*rest == '\0') { printf("error: load needs a file path\n"); return 1; }
        return load_from(rest) == 0 ? 0 : 1;
    }
    if (strcmp(tok, "free") == 0) { cmd_free(); return 0; }
    if (strcmp(tok, "validate") == 0) { cmd_validate(); return 0; }
    if (strcmp(tok, "min") == 0)      { cmd_min(); return 0; }
    if (strcmp(tok, "max") == 0)      { cmd_max(); return 0; }
    if (strcmp(tok, "balance") == 0)  { cmd_balance(); return 0; }
    if (strcmp(tok, "reverse") == 0)  { cmd_reverse(); return 0; }
    if (strcmp(tok, "sort") == 0)     { cmd_sort(); return 0; }
    if (strcmp(tok, "stats") == 0)    { cmd_stats(); return 0; }

    printf("error: unknown command '%s' (try 'help')\n", tok);
    return 1;
}

/* ============================================================
 * 7. Batch modes
 * ============================================================ */

/* Split a string by ';' and execute each piece.
 * Stops on the first error unless keep_going is set.
 * A quit inside the string stops the batch and returns 0. */
static int run_exec_string(char* s, int keep_going) {
    int rc = 0;
    char* start = s;
    for (;;) {
        char* semi = strchr(start, ';');
        if (semi != NULL) *semi = '\0';
        int r = execute_line(start);
        if (r == 2) return 0;
        if (r != 0) {
            rc = 1;
            if (!keep_going) return rc;
        }
        if (semi == NULL) break;
        start = semi + 1;
    }
    return rc;
}

/* Read a file line by line, execute each line.
 * Stops on the first error unless keep_going is set.
 * A quit inside the file stops the script and returns 0. */
static int run_script(const char* path, int keep_going) {
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        printf("error: cannot open '%s': %s\n", path, strerror(errno));
        return 1;
    }
    char line[256];
    int rc = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        int r = execute_line(line);
        if (r == 2) break;
        if (r != 0) {
            rc = 1;
            if (!keep_going) break;
        }
    }
    fclose(f);
    return rc;
}

static void usage(const char* prog) {
    printf("usage: %s [--exec \"cmd; cmd; ...\"] [--script FILE] [--keep-going]\n", prog);
    printf("       (no args: interactive mode)\n");
}

/* ============================================================
 * 8. main
 * ============================================================ */

int main(int argc, char** argv) {
    const char* exec_str = NULL;
    const char* script   = NULL;
    int keep_going = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--exec") == 0 && i + 1 < argc) {
            exec_str = argv[++i];
        } else if (strcmp(argv[i], "--script") == 0 && i + 1 < argc) {
            script = argv[++i];
        } else if (strcmp(argv[i], "--keep-going") == 0) {
            keep_going = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0]); return 0;
        } else {
            usage(argv[0]); return 1;
        }
    }

    if (exec_str != NULL) {
        char buf[1024];
        snprintf(buf, sizeof buf, "%s", exec_str);
        return run_exec_string(buf, keep_going);
    }
    if (script != NULL) {
        return run_script(script, keep_going);
    }

    char line[256];
    printf("algo repl. type 'help' for commands, 'quit' to exit.\n");
    for (;;) {
        printf("> ");
        fflush(stdout);
        if (fgets(line, sizeof line, stdin) == NULL) break;
        int r = execute_line(line);
        if (r == 2) break;
    }
    teardown();
    printf("bye\n");
    return 0;
}
