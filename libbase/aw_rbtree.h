#ifndef __AW_RBTREE_H__
#define __AW_RBTREE_H__

struct aw_rbtree_node_s {
    aw_key_t             key;
    aw_rbtree_node_t    *parent;
    aw_rbtree_node_t    *left;
    aw_rbtree_node_t    *right;
    aw_uchar_t           color;
    aw_uchar_t           usersigned;
};

typedef struct {
    aw_rbtree_node_t    *root;
    aw_rbtree_node_t     sentinel;
    aw_rbtree_node_t    *nil;
} aw_rbtree_t;

#define AW_RBTREE_RED       0
#define AW_RBTREE_BLK       1
#define AW_RBTREE_DBLK      2

#define aw_rbtree_node_init(tree, node)     \
    (node)->left = (tree)->nil;             \
    (node)->right = (tree)->nil;            \
    (node)->color = AW_RBTREE_RED;          \
    (node)->usersigned  = 0;

#define aw_rbtree_node_red(node) (node)->color = AW_RBTREE_RED;
#define aw_rbtree_node_blk(node) (node)->color = AW_RBTREE_BLK;
#define aw_rbtree_node_dblk(node) (node)->color = AW_RBTREE_DBLK;
#define aw_rbtree_node_isred(node) ((node)->color == AW_RBTREE_RED)
#define aw_rbtree_node_isblk(node) ((node)->color == AW_RBTREE_BLK)
#define aw_rbtree_node_isdblk(node) ((node)->color == AW_RBTREE_DBLK)

#define aw_rbtree_init(tree)                \
    (tree)->nil = &(tree)->sentinel;        \
    (tree)->sentinel.key = 0;               \
    (tree)->sentinel.left = (tree)->nil;    \
    (tree)->sentinel.right = (tree)->nil;   \
    (tree)->sentinel.usersigned  = 0;       \
    aw_rbtree_node_blk(&(tree)->sentinel);  \
    (tree)->root = (tree)->nil;

#define aw_rbtree_empty(tree)               \
    ((tree)->root == (tree)->nil)

#define aw_rbtree_entry(ptr, type, member)  \
    (type*)((char *)ptr - offsetof(type, member));

aw_rbtree_node_t *aw_rbtree_search(aw_rbtree_t *tree, aw_key_t key);
aw_rbtree_node_t *aw_rbtree_pop(aw_rbtree_t *tree, aw_key_t key);

aw_rbtree_node_t *aw_rbtree_min(aw_rbtree_t *tree, aw_rbtree_node_t *node);
aw_rbtree_node_t *aw_rbtree_max(aw_rbtree_t *tree, aw_rbtree_node_t *node);

aw_rbtree_node_t *aw_rbtree_popmin(aw_rbtree_t *tree, aw_rbtree_node_t *node);
aw_rbtree_node_t *aw_rbtree_popmax(aw_rbtree_t *tree, aw_rbtree_node_t *node);

aw_rbtree_node_t *aw_rbtree_predecessor(aw_rbtree_t *tree, aw_rbtree_node_t *node);
aw_rbtree_node_t *aw_rbtree_sucessor(aw_rbtree_t *tree, aw_rbtree_node_t *node);

aw_rbtree_node_t *aw_rbtree_poppredecessor(aw_rbtree_t *tree, aw_rbtree_node_t *node);
aw_rbtree_node_t *aw_rbtree_popsucessor(aw_rbtree_t *tree, aw_rbtree_node_t *node);

void aw_rbtree_insert(aw_rbtree_t *tree, aw_rbtree_node_t *node);
void aw_rbtree_delete(aw_rbtree_t *tree, aw_rbtree_node_t *node);

void aw_rbtree_print(aw_rbtree_t *tree, aw_rbtree_node_t *node, size_t indent);

#endif
