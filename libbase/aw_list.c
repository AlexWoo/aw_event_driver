#include <aw_core.h>

void
aw_list_insert(aw_list_t *list, aw_list_node_t *node,
               aw_list_node_t *newnode)
{
    aw_list_node_t *prev;

    prev = node->prev;

    prev->next = newnode;
    newnode->prev = prev;
    node->prev = newnode;
    newnode->next = node;
}

void
aw_list_delete(aw_list_t *list, aw_list_node_t *node)
{
    aw_list_node_t *prev, *next;

    if (aw_list_empty(list)) {  //empty list

        return;
    }

    prev = node->prev;
    next = node->next;

    prev->next = next;
    next->prev = prev;
}

void
aw_list_clear(aw_list_t *list)
{
    list->nil->next = list->nil;
    list->nil->prev = list->nil;
}

void
aw_list_pushhead(aw_list_t *list, aw_list_node_t *node)
{
    aw_list_insert(list, list->nil->next, node);
}

void
aw_list_pushtail(aw_list_t *list, aw_list_node_t *node)
{
    aw_list_insert(list, list->nil, node);
}

aw_list_node_t *
aw_list_pophead(aw_list_t *list)
{
    aw_list_node_t *node;

    if (aw_list_empty(list)) {  //empty list

        return NULL;
    }

    node = list->nil->next;
    aw_list_delete(list, node);

    return node;
}

aw_list_node_t *
aw_list_poptail(aw_list_t *list)
{
    aw_list_node_t *node;

    if (aw_list_empty(list)) {  //empty list

        return NULL;
    }

    node = list->nil->prev;
    aw_list_delete(list, node);

    return node;
}

aw_list_node_t *
aw_list_head(aw_list_t *list)
{
    aw_list_node_t *node;

    if (aw_list_empty(list)) {  //empty list

        return NULL;
    }

    node = list->nil->next;

    return node;
}

aw_list_node_t *
aw_list_tail(aw_list_t *list)
{
    aw_list_node_t *node;

    if (aw_list_empty(list)) {  //empty list

        return NULL;
    }

    node = list->nil->prev;

    return node;
}

#ifdef AW_UNIT_TEST
typedef struct {
    int                 i;
    aw_list_node_t      ln;
} Int;

typedef struct {
    float               f;
    aw_list_node_t      ln;
} Float;

void printnode(aw_list_node_t *p)
{
    Int            *pi;
    Float          *pf;

    if (p->usersigned == 1) {

        pi = aw_list_entry(p, Int, ln);
        printf("%d\t", pi->i);
    } else if (p->usersigned == 2) {

        pf = aw_list_entry(p, Float, ln);
        printf("%f\t", pf->f);
    }
}

void print(aw_list_t *list)
{
    aw_list_node_t *p;

    printf("###############################################\n");
    for (p = list->nil->next; p != list->nil; p = p->next) {

        printnode(p);
    }
    printf("\n");
}

int main()
{
    aw_list_t list;
    aw_list_init(&list);

    print(&list);

    Int i1;
    i1.i = 1;
    i1.ln.usersigned = 1;

    aw_list_pushhead(&list, &i1.ln);
    print(&list);

    Int i2;
    i2.i = 2;
    i2.ln.usersigned = 1;

    aw_list_pushhead(&list, &i2.ln);
    print(&list);

    Int i3;
    i3.i = 3;
    i3.ln.usersigned = 1;

    aw_list_insert(&list, &i1.ln, &i3.ln);
    print(&list);

    Float f1;
    f1.f = 0.1;
    f1.ln.usersigned = 2;

    aw_list_pushtail(&list, &f1.ln);
    print(&list);

    Float f2;
    f2.f = 0.2;
    f2.ln.usersigned = 2;

    aw_list_pushhead(&list, &f2.ln);
    print(&list);

    Float f3;
    f3.f = 0.3;
    f3.ln.usersigned = 2;

    aw_list_insert(&list, &i3.ln, &f3.ln);
    print(&list);

    aw_list_node_t *h = aw_list_head(&list);
    printf("the head of list is\n");
    printnode(h);
    printf("\n");
    
    aw_list_node_t *t = aw_list_tail(&list);
    printf("the tail of list is\n");
    printnode(t);
    printf("\n");

    aw_list_delete(&list, &i3.ln);
    print(&list);

    aw_list_node_t *p = aw_list_pophead(&list);
    printf("the head pop from list is\n");
    printnode(p);
    printf("\n");
    print(&list);

    p = aw_list_poptail(&list);
    printf("the tail pop from list is\n");
    printnode(p);
    printf("\n");
    print(&list);

    aw_list_clear(&list);
    print(&list);

    return 1;
}

#endif
