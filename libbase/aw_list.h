#ifndef __AW_LIST_H__
#define __AW_LIST_H__

struct aw_list_node_s {
    aw_list_node_t     *prev;
    aw_list_node_t     *next;
    aw_uchar_t          usersigned;
};

typedef struct {
    aw_list_node_t      sentinel;
    aw_list_node_t     *nil;
} aw_list_t;

#define aw_list_node_init(node)       \
    (node)->usersigned = 0;

#define aw_list_init(list)                  \
    aw_list_node_init(&(list)->sentinel)    \
    (list)->nil = &(list)->sentinel;        \
    (list)->nil->prev = (list)->nil;        \
    (list)->nil->next = (list)->nil;

#define aw_list_empty(list)                 \
    ((list)->nil->next == (list)->nil)

#define aw_list_entry(ptr, type, member)    \
    (type*)((char *)ptr - offsetof(type, member));

void aw_list_insert(aw_list_t *list, aw_list_node_t *node,
                    aw_list_node_t *newnode);  //insert newnode before node
void aw_list_delete(aw_list_t *list, aw_list_node_t *node);
void aw_list_clear(aw_list_t *list);

void aw_list_pushhead(aw_list_t *list, aw_list_node_t *node);
void aw_list_pushtail(aw_list_t *list, aw_list_node_t *node);

aw_list_node_t *aw_list_pophead(aw_list_t *list);
aw_list_node_t *aw_list_poptail(aw_list_t *list);

aw_list_node_t *aw_list_head(aw_list_t *list);
aw_list_node_t *aw_list_tail(aw_list_t *list);

#endif
