#include <aw_core.h>

static void aw_rbtree_rotateleft(aw_rbtree_t *tree, aw_rbtree_node_t *node);
static void aw_rbtree_rotateright(aw_rbtree_t *tree, aw_rbtree_node_t *node);
static void aw_rbtree_transplant(aw_rbtree_t *tree, aw_rbtree_node_t *node,
                                 aw_rbtree_node_t *newnode);
static void aw_rbtree_insert_fixup(aw_rbtree_t *tree, aw_rbtree_node_t *node);
static void aw_rbtree_delete_fixup(aw_rbtree_t *tree, aw_rbtree_node_t *node);

aw_rbtree_node_t *
aw_rbtree_search(aw_rbtree_t *tree, aw_key_t key)
{
    aw_rbtree_node_t *p;

    p = tree->root;

    while (p != tree->nil) {

        if (key == p->key) {

            return p;
        } else if (key < p->key) {

            p = p->left;
        } else {

            p = p->right;
        }
    }

    return NULL;
}

aw_rbtree_node_t *
aw_rbtree_min(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p = node;

    while (p->left != tree->nil) {

        p = p->left;
    }

    return p;
}

aw_rbtree_node_t *
aw_rbtree_max(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p = node;

    while (p->right != tree->nil) {

        p = p->right;
    }

    return p;
}

aw_rbtree_node_t *
aw_rbtree_popmin(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p = aw_rbtree_min(tree, node);
    aw_rbtree_delete(tree, p);

    return p;
}

aw_rbtree_node_t *
aw_rbtree_popmax(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p = aw_rbtree_max(tree, node);
    aw_rbtree_delete(tree, p);

    return p;
}

aw_rbtree_node_t *
aw_rbtree_predecessor(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p = node;

    if (p->left != tree->nil) {  //node have left sub tree

        return aw_rbtree_max(tree, p->left); //return the maxinum node in left sub tree
    }

    p = node->parent;
    while (p != tree->nil) {

        if (node == p->right) {

            return p;
        }
        node = p;
        p = node->parent;
    }

    return NULL;
}

aw_rbtree_node_t *
aw_rbtree_sucessor(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p = node;

    if (p->right != tree->nil) {  //node have right sub tree

        return aw_rbtree_min(tree, p->right); //return the mininum node in right sub tree
    }

    p = node->parent;
    while (p != tree->nil) {

        if (node == p->left) {

            return p;
        }
        node = p;
        p = node->parent;
    }

    return NULL;
}

void
aw_rbtree_insert(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *prev, *next;

    if (tree->root == tree->nil) { //tree is empty

        aw_rbtree_node_init(tree, node);
        tree->root = node;
        node->parent = tree->nil;
        aw_rbtree_node_blk(node);

        return;
    }

    next = tree->root;

    while (next != tree->nil) {

        prev = next;
        if (node->key > next->key) {

            next = next->right;
        } else {

            next = next->left;
        }
    }

    aw_rbtree_node_init(tree, node);
    node->parent = prev;
    if (node->key < prev->key) {

        prev->left = node;
    } else {

        prev->right = node;
    }

    aw_rbtree_insert_fixup(tree, node);

    aw_rbtree_node_blk(tree->root);
}

void
aw_rbtree_delete(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *px, *py;
    aw_uchar_t        xcolor;

    px = node;

    if (px->left == tree->nil) {

        xcolor = px->color;
        py = px->right;
        aw_rbtree_transplant(tree, px, py);
    } else if (px->right == tree->nil) {

        xcolor = px->color;
        py = px->left;
        aw_rbtree_transplant(tree, px, py);
    } else {

        px = aw_rbtree_sucessor(tree, px);
        //px's left tree must be tree->nil
        xcolor = px->color;
        py = px->right;
        aw_rbtree_transplant(tree, px, py);
        //replace px to node
        px->color = node->color;
        px->left = node->left;
        px->left->parent = px;
        px->right = node->right;
        px->right->parent = px;
        px->parent = node->parent;
        if (node->parent == tree->nil) {

            tree->root = px;
        } else if (node == node->parent->left) {

            node->parent->left = px;
        } else {

            node->parent->right = px;
        }
    }

    //if px is red, transplant py to px, no need to fixup
    if (xcolor == AW_RBTREE_BLK) {

        ++py->color;
        //if py is red, color it to black, no need to fixup
        if (aw_rbtree_node_isdblk(py)) {

            aw_rbtree_delete_fixup(tree, py);
        }
    }

    aw_rbtree_node_blk(tree->root);
}

static void
aw_rbtree_rotateleft(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p, *g;

    if (node->parent == tree->nil) {

        return;
    }

    p = node->parent;
    g = p->parent;

    if (p->left == node) {

        return;
    }

    p->right = node->left;
    node->left->parent = p;
    node->left = p;
    p->parent = node;
    node->parent = g;
    if (g == tree->nil) {  //p is the root of tree

        tree->root = node;
    } else if (g->left == p) {

        g->left = node;
    } else {

        g->right = node;
    }
}

static void
aw_rbtree_rotateright(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p, *g;

    if (node->parent == tree->nil) {

        return;
    }

    p = node->parent;
    g = p->parent;

    if (p->right == node) {

        return;
    }

    p->left = node->right;
    node->right->parent = p;
    node->right = p;
    p->parent = node;
    node->parent = g;
    if (g == tree->nil) {  //p is the root of tree

        tree->root = node;
    } else if (g->left == p) {

        g->left = node;
    } else {

        g->right = node;
    }
}

static void aw_rbtree_transplant(aw_rbtree_t *tree, aw_rbtree_node_t *node,
                                 aw_rbtree_node_t *newnode)
{
    aw_rbtree_node_t *p;

    p = node->parent;

    if (p == tree->nil) {  //node is the root of tree

        tree->root = newnode;
    } else if (p->left == node) {

        p->left = newnode;
    } else {

        p->right = newnode;
    }

    newnode->parent = p;
}

static void
aw_rbtree_insert_fixup(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t *p;

    while (tree->root != node && aw_rbtree_node_isred(node)) {

        p = node->parent;

        if (aw_rbtree_node_isred(p)) {
            if (p->right == node && p->parent->left == p) {

                aw_rbtree_rotateleft(tree, node);
            } else if (p->left == node && p->parent->right == p) {

                aw_rbtree_rotateright(tree, node);
            } else {

                node = p;
            }

            p = node->parent;

            if (p->right == node
                && aw_rbtree_node_isred(node->right)) {

                aw_rbtree_rotateleft(tree, node);
                aw_rbtree_node_blk(node);
                aw_rbtree_node_red(p);
            } else if (p->left == node && aw_rbtree_node_isred(node->left)) {

                aw_rbtree_rotateright(tree, node);
                aw_rbtree_node_blk(node);
                aw_rbtree_node_red(p);
            }
        } else {

            node = p;
        }

        if (aw_rbtree_node_isred(node->left)
            && aw_rbtree_node_isred(node->right)) {

            aw_rbtree_node_red(node);
            aw_rbtree_node_blk(node->left);
            aw_rbtree_node_blk(node->right);
        }        
    }
}

#define aw_rbtree_delete_init(tree, n, pa, pb, pc)      \
    pb = n;                                             \
    pa = pb->parent;                                    \
    if (pa == tree->nil) {                              \
                                                        \
        pc = pb;                                        \
    }                                                   \
    else if (pb == pa->left) {                          \
                                                        \
        pc = pa->right;                                 \
    } else {                                            \
                                                        \
        pc = pa->left;                                  \
    }

static void
aw_rbtree_delete_fixup(aw_rbtree_t *tree, aw_rbtree_node_t *node)
{
    aw_rbtree_node_t    *pa, *pb, *pc, *pd;
    aw_uchar_t           acolor;

    aw_rbtree_delete_init(tree, node, pa, pb, pc);

    while (aw_rbtree_node_isdblk(pb) && pb->parent != tree->nil) {

        if (aw_rbtree_node_isred(pc)) {

            if (pa->right == pc) {

                aw_rbtree_rotateleft(tree, pc);
            } else {

                aw_rbtree_rotateright(tree, pc);
            }
            aw_rbtree_node_red(pa);
            aw_rbtree_node_blk(pc);
            aw_rbtree_delete_init(tree, pb, pa, pb, pc);
        }

        if (aw_rbtree_node_isblk(pc)) {

            if ((aw_rbtree_node_isblk(pc->left) || pc->left == tree->nil)
                && (aw_rbtree_node_isblk(pc->right) || pc->right == tree->nil)) {

                aw_rbtree_node_blk(pb);
                aw_rbtree_node_red(pc);
                ++pa->color; //pa is possible double black node
                aw_rbtree_delete_init(tree, pa, pa, pb, pc);
                continue;
            }

            acolor = pa->color;
            if (pa->right == pc && aw_rbtree_node_isred(pc->right)) {

                pd = pc->right;
                aw_rbtree_rotateleft(tree, pc);
                aw_rbtree_node_blk(pa);
                aw_rbtree_node_blk(pb);
                aw_rbtree_node_blk(pd);
                pc->color = acolor;
            } else if (pa->left == pc && aw_rbtree_node_isred(pc->left)) {

                pd = pc->left;
                aw_rbtree_rotateright(tree, pc);
                aw_rbtree_node_blk(pa);
                aw_rbtree_node_blk(pb);
                aw_rbtree_node_blk(pd);
                pc->color = acolor;
            } else if (pa->right == pc && aw_rbtree_node_isred(pc->left)) {

                pd = pc->left;
                aw_rbtree_rotateright(tree, pd);
                aw_rbtree_rotateleft(tree, pd);
                aw_rbtree_node_blk(pa);
                aw_rbtree_node_blk(pb);
                aw_rbtree_node_blk(pc);
                pd->color = acolor;
            } else{

                pd = pc->right;
                aw_rbtree_rotateleft(tree, pd);
                aw_rbtree_rotateright(tree, pd);
                aw_rbtree_node_blk(pa);
                aw_rbtree_node_blk(pb);
                aw_rbtree_node_blk(pc);
                pd->color = acolor;
            }

            break;
        }
    }
}

#define aw_rbtree_print_indent(indent)  \
    size_t i;                           \
    for (i = 0; i < indent; ++i) {      \
                                        \
        printf(" ");                    \
    }                                   \
    printf("|--");

#define aw_rbtree_print_color(node)     \
    ((node)->color == AW_RBTREE_RED)? "R": ((node)->color == AW_RBTREE_BLK)? "B": "DB"

#define aw_rbtree_print_branch(tree, node)  \
    ((tree)->root == (node))? "ROOT": ((node)->parent->left == (node))? "LEFT": "RIGHT"

void
aw_rbtree_print(aw_rbtree_t *tree, aw_rbtree_node_t *node, size_t indent)
{
    if (node != tree->nil) {

        aw_rbtree_print_indent(indent);
        printf("[%s][%s]%lu\n",
            aw_rbtree_print_branch(tree, node),
            aw_rbtree_print_color(node),
            node->key);
        aw_rbtree_print(tree, node->left, indent + 1);
        aw_rbtree_print(tree, node->right, indent + 1);
    }
}

#ifdef AW_UNIT_TEST

int main()
{
    aw_rbtree_t rbt;
    aw_rbtree_init(&rbt);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node1;
    aw_rbtree_node_init(&rbt, &node1);
    node1.key = 100;
    aw_rbtree_insert(&rbt, &node1);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node2;
    aw_rbtree_node_init(&rbt, &node2);
    node2.key = 11;
    aw_rbtree_insert(&rbt, &node2);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node3;
    aw_rbtree_node_init(&rbt, &node3);
    node3.key = 7;
    aw_rbtree_insert(&rbt, &node3);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node4;
    aw_rbtree_node_init(&rbt, &node4);
    node4.key = 102;
    aw_rbtree_insert(&rbt, &node4);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node5;
    aw_rbtree_node_init(&rbt, &node5);
    node5.key = 4;
    aw_rbtree_insert(&rbt, &node5);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node6;
    aw_rbtree_node_init(&rbt, &node6);
    node6.key = 5;
    aw_rbtree_insert(&rbt, &node6);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node7;
    aw_rbtree_node_init(&rbt, &node7);
    node7.key = 101;
    aw_rbtree_insert(&rbt, &node7);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");


    aw_rbtree_node_t node8;
    aw_rbtree_node_init(&rbt, &node8);
    node8.key = 1000;
    aw_rbtree_insert(&rbt, &node8);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node9;
    aw_rbtree_node_init(&rbt, &node9);
    node9.key = 900;
    aw_rbtree_insert(&rbt, &node9);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node10;
    aw_rbtree_node_init(&rbt, &node10);
    node10.key = 905;
    aw_rbtree_insert(&rbt, &node10);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node11;
    aw_rbtree_node_init(&rbt, &node11);
    node11.key = 38;
    aw_rbtree_insert(&rbt, &node11);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node12;
    aw_rbtree_node_init(&rbt, &node12);
    node12.key = 44;
    aw_rbtree_insert(&rbt, &node12);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");


    aw_rbtree_node_t node13;
    aw_rbtree_node_init(&rbt, &node13);
    node13.key = 891;
    aw_rbtree_insert(&rbt, &node13);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node14;
    aw_rbtree_node_init(&rbt, &node14);
    node14.key = 661;
    aw_rbtree_insert(&rbt, &node14);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node15;
    aw_rbtree_node_init(&rbt, &node15);
    node15.key = 907;
    aw_rbtree_insert(&rbt, &node15);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t* node_min = aw_rbtree_min(&rbt, rbt.root);
    aw_rbtree_node_t* node_max = aw_rbtree_max(&rbt, rbt.root);
    printf("min: %lu, max: %lu\n", node_min->key, node_max->key);
    printf("---------------------------------------------\n");

    node_min = aw_rbtree_min(&rbt, &node8);
    node_max = aw_rbtree_max(&rbt, &node8);
    printf("min: %lu, max: %lu\n", node_min->key, node_max->key);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t* node_pre = aw_rbtree_predecessor(&rbt, &node7);
    aw_rbtree_node_t* node_suc = aw_rbtree_sucessor(&rbt, &node7);
    printf("predecessor: %lu, sucessor: %lu\n", node_pre->key, node_suc->key);
    printf("---------------------------------------------\n");

    node_pre = aw_rbtree_predecessor(&rbt, &node3);
    node_suc = aw_rbtree_sucessor(&rbt, &node4);
    printf("predecessor: %lu, sucessor: %lu\n", node_pre->key, node_suc->key);
    printf("---------------------------------------------\n");

    node_pre = aw_rbtree_predecessor(&rbt, &node5);
    node_suc = aw_rbtree_sucessor(&rbt, &node8);
    printf("node_pre: %p, node_suc: %p\n", node_pre, node_suc);
    printf("---------------------------------------------\n");


    //38 ×óºÚÒ¶, ÐÖºÚ, Ë«Ö¶ºÚ, ¸¸ºÚ
    aw_rbtree_delete(&rbt, &node11);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    //1000 ÓÒºÚÒ¶, ÐÖºÚ, Ë«Ö¶ºÚ, ¸¸ºì
    aw_rbtree_delete(&rbt, &node8);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    //44 µ¥ÓÒºìÖ¦
    aw_rbtree_delete(&rbt, &node12);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");
    
    //907 µ¥×óºìÖ¦
    aw_rbtree_delete(&rbt, &node15);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    //101 Ë«ºÚÖ¦
    aw_rbtree_delete(&rbt, &node7);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    //905 ÐÖºÚ, ½üÖ¶ºì
    aw_rbtree_delete(&rbt, &node10);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    //891 ÐÖºÚ, Ô¶Ö¶ºì
    aw_rbtree_delete(&rbt, &node13);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    aw_rbtree_node_t node16;
    aw_rbtree_node_init(&rbt, &node16);
    node16.key = 777;
    aw_rbtree_insert(&rbt, &node16);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    //100 ÐÖºì
    aw_rbtree_delete(&rbt, &node1);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    node_min = aw_rbtree_popmin(&rbt, rbt.root);
    printf("min: %lu\n", node_min->key);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    node_max = aw_rbtree_popmax(&rbt, rbt.root);
    printf("max: %lu\n", node_max->key);
    aw_rbtree_print(&rbt, rbt.root, 0);
    printf("---------------------------------------------\n");

    return 0;
}
#endif
