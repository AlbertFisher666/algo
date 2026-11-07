#ifndef ALGO_TREE_HPP
#define ALGO_TREE_HPP

/* tree.hpp - binary search tree contract. 80s C-flavoured C++. */

struct TreeNode {
    int key;
    TreeNode* left;
    TreeNode* right;
};

struct Tree {
    TreeNode* root;
    int size;
};

/* init: root=NULL, size=0 */
void tree_init(Tree* t);

/* recursive free of all nodes */
void tree_free(Tree* t);

/* insert key; 0 on OK, -1 on error/duplicate */
int tree_insert(Tree* t, int key);

/* find key; 1 if present, 0 otherwise */
int tree_find(const Tree* t, int key);

/* remove key; 0 on OK, -1 if not found */
int tree_remove(Tree* t, int key);

/* height of tree; 0 for empty */
int tree_height(const Tree* t);

/* in-order print */
void tree_print(const Tree* t);

#endif /* ALGO_TREE_HPP */
