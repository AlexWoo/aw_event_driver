#ifndef __AW_POOL_H__
#define __AW_POOL_H__

typedef     void *(*aw_alloc_pt)(size_t size);
typedef     void (*aw_dealloc_pt)(void  *ptr);

typedef struct {
    aw_uchar_t             *start;
    aw_uchar_t             *end;
    aw_uchar_t             *current;
    aw_list_node_t          lst_node;
} aw_pool_block_t;

typedef struct {
    aw_uchar_t             *start;
    aw_rbtree_node_t        rbt_node;
} aw_pool_large_t;

typedef struct {
    aw_list_t               block_lst;
    aw_uint32_t             n_block;
    size_t                  block_size;

    aw_rbtree_t             large_rbt;
    aw_uint32_t             n_large;
    size_t                  large_size;

    aw_alloc_pt             alloc_handler;
    aw_dealloc_pt           dealloc_handler;
} aw_pool_t;

aw_pool_t *aw_pool_create(size_t blocksize, size_t largesize,
                          aw_alloc_pt alloc_handler,
                          aw_dealloc_pt dealloc_handler);
void *aw_pool_malloc(aw_pool_t *pool, size_t size);
void *aw_pool_calloc(aw_pool_t *pool, size_t size);
void  aw_pool_free(aw_pool_t *pool, void *ptr);
void  aw_pool_destroy(aw_pool_t *pool);

#endif
