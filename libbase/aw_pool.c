#include <aw_core.h>

static aw_pool_block_t *aw_pool_create_block(aw_pool_t *pool);
static aw_pool_large_t *aw_pool_create_large(aw_pool_t *pool, size_t size);
static void *aw_pool_malloc_block(aw_pool_t *pool, size_t size);
static void *aw_pool_malloc_large(aw_pool_t *pool, size_t size);
static void aw_pool_destroy_block(aw_pool_t *pool);
static void aw_pool_destroy_large(aw_pool_t *pool);

#define aw_align_mem_ptr(p)      \
    (u_char *) (((unsigned long) (p) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1))
#define aw_align_mem_size(p)     \
    (((unsigned long) (p) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1))

aw_pool_t *aw_pool_create(size_t blocksize, size_t largesize,
                          aw_alloc_pt alloc_handler,
                          aw_dealloc_pt dealloc_handler)
{
    aw_pool_t          *pool;
    aw_pool_block_t    *block;

    pool = alloc_handler(sizeof(aw_pool_t));
    if (NULL == pool) {

        return NULL;
    }

    aw_list_init(&pool->block_lst);
    pool->block_size = blocksize;

    aw_rbtree_init(&pool->large_rbt);
    pool->n_large = 0;
    pool->large_size = largesize;

    pool->alloc_handler = alloc_handler;
    pool->dealloc_handler = dealloc_handler;

    block = aw_pool_create_block(pool);
    if (NULL == block) {

        dealloc_handler(pool);
        return NULL;
    }

    return pool;
}

void *aw_pool_malloc(aw_pool_t *pool, size_t size)
{
    if (size > pool->large_size) {

        return aw_pool_malloc_large(pool, size);
    } else {

        return aw_pool_malloc_block(pool, size);
    }
}

void *aw_pool_calloc(aw_pool_t *pool, size_t size)
{
    void *p;

    p = aw_pool_malloc(pool, size);
    memset(p, 0, size);

    return p;
}

void  aw_pool_free(aw_pool_t *pool, void *ptr)
{
    //pool can only free ptr in large_rbt
    aw_pool_large_t        *large;
    aw_rbtree_node_t       *large_node;
    aw_key_t                key;

    key = aw_ptr_key(ptr);
    large_node = aw_rbtree_search(&pool->large_rbt, key);
    if (NULL == large_node) {

        return;
    }

    aw_rbtree_delete(&pool->large_rbt, large_node);
    large = aw_rbtree_entry(large_node, aw_pool_large_t, rbt_node);
    pool->dealloc_handler(large);
    --pool->n_large;
}

void  aw_pool_destroy(aw_pool_t *pool)
{
    aw_pool_destroy_block(pool);
    aw_pool_destroy_large(pool);

    pool->dealloc_handler(pool);
}

static aw_pool_block_t *
aw_pool_create_block(aw_pool_t *pool)
{
    aw_pool_block_t    *pool_block;
    size_t              pool_block_size;
    size_t              size_block_struct;

    size_block_struct = aw_align_mem_size(sizeof(aw_pool_block_t));
    pool_block_size = size_block_struct + pool->block_size;
    pool_block = pool->alloc_handler(pool_block_size);
    if (NULL == pool_block) {

        return NULL;
    }

    pool_block->start = (aw_uchar_t *) pool_block + size_block_struct;
    pool_block->end = (aw_uchar_t *) pool_block + pool_block_size;
    pool_block->current = pool_block->start;
    aw_list_node_init(&pool_block->lst_node);

    aw_list_pushtail(&pool->block_lst, &pool_block->lst_node);
    ++pool->n_block;

    return pool_block;
}

static aw_pool_large_t *
aw_pool_create_large(aw_pool_t *pool, size_t size)
{
    aw_pool_large_t    *pool_large;
    size_t              pool_large_size;
    size_t              size_large_struct;
    void               *ptr;

    size_large_struct = aw_align_mem_size(sizeof(aw_pool_large_t));
    pool_large_size = size_large_struct + size;
    pool_large = pool->alloc_handler(pool_large_size);
    if (NULL == pool_large) {

        return NULL;
    }

    pool_large->start = (aw_uchar_t *) pool_large + size_large_struct;
    ptr = aw_align_mem_ptr(pool_large->start);
    aw_rbtree_node_init(&pool->large_rbt, &pool_large->rbt_node);
    pool_large->rbt_node.key = aw_ptr_key(ptr);

    aw_rbtree_insert(&pool->large_rbt, &pool_large->rbt_node);
    ++pool->n_large;

    return pool_large;
}

static void *
aw_pool_malloc_block(aw_pool_t *pool, size_t size)
{
    aw_pool_block_t    *block;
    aw_list_node_t     *lst_node;
    void               *ptr;

    lst_node = aw_list_tail(&pool->block_lst);
    block = aw_list_entry(lst_node, aw_pool_block_t, lst_node);

    ptr = aw_align_mem_ptr(block->current);
    if ((aw_uchar_t *) ptr + size > block->end) { //need to create a new block

        block = aw_pool_create_block(pool);
        if (NULL == block) {

            return NULL;
        }
        ptr = aw_align_mem_ptr(block->current);
    }

    block->current = (aw_uchar_t *) ptr + size;

    return ptr;
}

static void *
aw_pool_malloc_large(aw_pool_t *pool, size_t size)
{
    aw_pool_large_t    *large;
    void               *ptr;

    large = aw_pool_create_large(pool, size);
    if (NULL == large) {

        return NULL;
    }

    ptr = aw_align_mem_ptr(large->start);

    return ptr;
}

static void
aw_pool_destroy_block(aw_pool_t *pool)
{
    aw_list_node_t     *lst_node;
    aw_pool_block_t    *block;

    while (!aw_list_empty(&pool->block_lst)) {

        lst_node = aw_list_pophead(&pool->block_lst);
        block = aw_list_entry(lst_node, aw_pool_block_t, lst_node);
        pool->dealloc_handler(block);
    }
}

static void
aw_pool_destroy_large(aw_pool_t *pool)
{
    aw_rbtree_node_t    *rbt_node;
    aw_pool_large_t     *large;

    while (!aw_rbtree_empty(&pool->large_rbt)) {

        rbt_node = aw_rbtree_popmin(&pool->large_rbt, pool->large_rbt.root);
        large = aw_rbtree_entry(rbt_node, aw_pool_large_t, rbt_node);
        pool->dealloc_handler(large);
    }
}

#ifdef AW_POOL_TEST
void aw_list_print(aw_list_t *list)
{
    aw_list_node_t      *p;
    aw_pool_block_t     *block;

    for (p = list->nil->next; p != list->nil; p = p->next) {

        block = aw_list_entry(p, aw_pool_block_t, lst_node);
        printf("\t %p(s:%p, e:%p, c:%p)\n",
               block, block->start, block->end, block->current);
    }
    printf("\n");
}

void aw_pool_print(aw_pool_t *pool)
{
    printf("blocksize: %lu\n", (unsigned long) pool->block_size);
    printf("largesize: %lu\n", (unsigned long) pool->large_size);
    printf("n_block: %u\n", pool->n_block);
    aw_list_print(&pool->block_lst);
    printf("n_large: %u\n", pool->n_large);
    aw_rbtree_print(&pool->large_rbt, pool->large_rbt.root, 4);
}

int main()
{
    aw_pool_t       *pool;
    void            *b1, *b2, *b3, *b4, *b5;
    void            *l1, *l2, *l3;


    pool = aw_pool_create(128, 32, malloc, free);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);

    b1 = aw_pool_malloc(pool, 16);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);
    printf("b1, %p\n", b1);

    b2 = aw_pool_calloc(pool, 24);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);
    printf("b2, %p\n", b2);

    b3 = aw_pool_malloc(pool, 32);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);
    printf("b3, %p\n", b3);

    b4 = aw_pool_malloc(pool, 32);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);
    printf("b4, %p\n", b4);
    
    b5 = aw_pool_malloc(pool, 32);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);
    printf("b5, %p\n", b5);

    l1 = aw_pool_malloc(pool, 64);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);
    printf("l1, %p\n", l1);

    l2 = aw_pool_malloc(pool, 128);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);
    printf("l2, %p\n", l2);

    l3 = aw_pool_malloc(pool, 1024);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);
    printf("l3, %p\n", l3);

    aw_pool_free(pool, b1);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);

    aw_pool_free(pool, l3);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);

    aw_pool_destroy(pool);
    printf("-----------------------------------------\n");
    aw_pool_print(pool);

    return 0;
}

#endif
