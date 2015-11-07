 #include <aw_core.h>

typedef struct {
    void                   *mptr;               //to memory
    void                   *bptr;               //to block
    aw_rbtree_node_t        inuse_rbt_node;     //link in aw_mem_alloc_t.mem_inuse_rbt, key is mptr
    aw_list_node_t          free_lst_node;      //link in aw_mem_block_t.mem_free_lst
} aw_mem_block_manager_node_t;

typedef struct {
    size_t                  slot_size;          //mem size to allocate in this slot
    aw_rbtree_t             slot_tid_rbt;       //block group by tid
} aw_mem_block_slot_t;

typedef struct {
    aw_rbtree_node_t        slot_tid_rbt_node;  //link in aw_mem_block_slot_t.slot_tid_rbt
    aw_list_t               slot_block_lst;     //link block alloct to same thread
    aw_uint32_t             n_mem_block;
    aw_mem_block_slot_t    *slot;
} aw_mem_block_slot_tid_t;

typedef struct {
    aw_tid_t                    tid;                //block bind to thread, if not bind, set to 0
    aw_list_node_t              block_lst_node;     //link in aw_mem_block_slot_tid_t.slot_block_lst
    aw_list_t                   mem_free_lst;       //mem can allocate in this block
    aw_uint32_t                 n_mem_free;
    aw_mem_block_slot_tid_t    *slot_tid;
} aw_mem_block_t;

typedef struct {
    aw_rbtree_node_t        large_rbt_node;     //link to aw_mem_alloc_t.
} aw_mem_large_t;

typedef struct {
    aw_pool_t              *mem_manager_pool;
    aw_pool_t              *mem_pool;

    aw_mem_block_slot_t    *mem_block_slot;
    aw_uint32_t             n_mem_block_slot;

    size_t                  mem_large_size;

    aw_uint32_t             n_mem_inuse;
    aw_rbtree_t             mem_inuse_rbt;      //node usersigned set to 1 represent large
} aw_mem_alloc_t;

static aw_mem_alloc_t   *aw_mem_allocator = NULL;

static int aw_alloc_init();

static aw_uint8_t aw_get_slot_index(size_t size);
static void aw_mem_block_manager_node_init(aw_mem_block_manager_node_t *mnode,
                                           void *mptr, void *bptr);
static aw_mem_block_t *aw_mem_block_create(aw_tid_t tid, size_t size);
static aw_mem_block_t *aw_mem_block_fetch(size_t size);
static aw_mem_block_slot_tid_t *aw_mem_block_slot_tid_create(aw_tid_t tid, size_t size);
static int aw_mem_block_slot_init(aw_mem_block_slot_t *slot, size_t size);

static aw_mem_block_slot_tid_t *aw_mem_block_slot_tid_get(aw_tid_t tid, size_t size);
static aw_mem_block_t *aw_mem_block_idle(aw_tid_t tid, size_t size);

static void *aw_mem_block_malloc(size_t size);
static void  aw_mem_block_free(aw_rbtree_node_t *rbt_node);

static void *aw_mem_large_malloc(size_t size);
static void  aw_mem_large_free(aw_rbtree_node_t *rbt_node);


void *aw_malloc(size_t size)
{
    if (NULL == aw_mem_allocator) {

        if (AW_ERROR == aw_alloc_init()) {

            exit(1);
        }
    }

    if (size > aw_mem_allocator->mem_large_size) {

        return aw_mem_large_malloc(size);
    } else {

        return aw_mem_block_malloc(size);
    }
}

void *aw_calloc(size_t nmemb, size_t size)
{
    void *ptr = aw_malloc(nmemb * size);
    memset(ptr, 0, nmemb * size);

    return ptr;
}

void *aw_realloc(void *ptr, size_t size)
{
    void *nptr = aw_malloc(size);
    memcpy(nptr, ptr, size);

    return nptr;
}

void  aw_free(void *ptr)
{
    aw_key_t                        key;
    aw_rbtree_node_t               *rbt_node;

    key = aw_ptr_key(ptr);
    rbt_node = aw_rbtree_search(&aw_mem_allocator->mem_inuse_rbt, key);
    if (NULL == rbt_node) { //free a memory not allocat

        return;
    }

    if (1 == rbt_node->usersigned) { //node is in aw_mem_large_t

        aw_mem_large_free(rbt_node);
    } else {

        aw_mem_block_free(rbt_node);

    }
}



static int
aw_alloc_init()
{
    aw_pool_t      *mem_pool;
    aw_pool_t      *mem_manager_pool;
    size_t          mem_pool_size;
    size_t          mem_manager_pool_size;
    size_t          mem_size;
    aw_uint32_t     mem_n_slot, i;

    mem_pool_size = AW_MEM_POOL_SIZE;
    mem_manager_pool_size = AW_MEM_MANAGER_POOL_SIZE;
    mem_size = AW_MEM_BLOCK_FIRST_SLOTSIZE;
    mem_n_slot = AW_MEM_N_BLOCK_SLOT;

    mem_pool = aw_pool_create(mem_pool_size, mem_pool_size, malloc, free);
    if (NULL == mem_pool) {

        return AW_ERROR;
    }

    mem_manager_pool = aw_pool_create(mem_manager_pool_size,
                                      mem_manager_pool_size,
                                      malloc, free);
    if (NULL == mem_manager_pool) {

        aw_pool_destroy(mem_pool);
        return AW_ERROR;
    }

    aw_mem_allocator = aw_pool_calloc(mem_manager_pool, sizeof(aw_mem_alloc_t));
    if (NULL == aw_mem_allocator) {

        goto failed;
    }

    aw_mem_allocator->mem_manager_pool = mem_manager_pool;
    aw_mem_allocator->mem_pool = mem_pool;

    //init mem_block
    aw_mem_allocator->mem_block_slot
        = aw_pool_calloc(mem_manager_pool, mem_n_slot * sizeof(aw_mem_block_slot_t));
    if (NULL == aw_mem_allocator->mem_block_slot) {

        goto failed;
    }
    for (i = 0; i < mem_n_slot; ++i) {

        if (AW_ERROR ==
            aw_mem_block_slot_init(&aw_mem_allocator->mem_block_slot[i], mem_size)) {

            goto failed;
        }
        mem_size = mem_size << 1;
    }
    aw_mem_allocator->n_mem_block_slot = mem_n_slot;

    aw_rbtree_init(&aw_mem_allocator->mem_inuse_rbt);
    aw_mem_allocator->n_mem_inuse = 0;

    return AW_OK;

failed:
    aw_pool_destroy(mem_pool);
    aw_pool_destroy(mem_manager_pool);

    return AW_ERROR;
}

static aw_uint8_t
aw_get_slot_index(size_t size)
{
    int idx = 0;

    while (1) {

        if ((size >> idx) <= AW_MEM_BLOCK_FIRST_SLOTSIZE) {

            return idx;
        }
        ++idx;
    }
}

/*
 * typedef struct {
 *     void                   *mptr;               //to memory
 *     void                   *bptr;               //to block
 *     aw_rbtree_node_t        inuse_rbt_node;     //link in aw_mem_alloc_t.mem_inuse_rbt, key is mptr
 *     aw_list_node_t          free_lst_node;      //link in aw_mem_block_t.mem_free_lst
 * } aw_mem_block_manager_node_t;
 */
static void
aw_mem_block_manager_node_init(aw_mem_block_manager_node_t *mnode,
                               void *mptr, void *bptr)
{
    mnode->mptr = mptr;
    mnode->bptr = bptr;
    aw_rbtree_node_init(&aw_mem_allocator->mem_inuse_rbt, &mnode->inuse_rbt_node);
    mnode->inuse_rbt_node.key = aw_ptr_key(mnode->mptr);
    aw_list_node_init(&mnode->free_lst_node);
}

/*
 * typedef struct {
 *     aw_tid_t                tid;                //block bind to thread, if not bind, set to 0
 *     aw_list_node_t          block_lst_node;     //link in aw_mem_block_slot_tid_t.slot_block_lst
 *     aw_list_t               mem_free_lst;       //mem can allocate in this block
 *     aw_uint32_t             n_mem_free;
 * } aw_mem_block_t;
 */
static aw_mem_block_t *
aw_mem_block_create(aw_tid_t tid, size_t size)
{
    aw_mem_block_t                 *mblock;
    aw_mem_block_manager_node_t    *pmnode;
    aw_pool_t                      *mem_pool, *manager_pool;
    size_t                          mblock_size;
    aw_uint32_t                     n_mem, i;
    void                           *mptr, *bptr;

    mem_pool = aw_mem_allocator->mem_pool;
    manager_pool = aw_mem_allocator->mem_manager_pool;

    mblock_size = sizeof(aw_mem_block_t) + AW_MEM_BLOCK_SIZE;
    n_mem = AW_MEM_BLOCK_SIZE / size;

    mblock = aw_pool_calloc(mem_pool, mblock_size);
    if (NULL == mblock) {

        return NULL;
    }
    mblock->tid = tid;
    aw_list_node_init(&mblock->block_lst_node);
    aw_list_init(&mblock->mem_free_lst);
    mblock->n_mem_free = n_mem;

    pmnode = aw_pool_calloc(manager_pool, n_mem * sizeof(aw_mem_block_manager_node_t));
    if (NULL == pmnode) {

        aw_pool_free(mem_pool, mblock);
        return NULL;
    }

    mptr = (aw_uchar_t *) mblock + sizeof(aw_mem_block_t);
    bptr = mblock;

    for (i = 0; i < n_mem; ++i) {

        aw_mem_block_manager_node_init(&pmnode[i], mptr, bptr);
        aw_list_pushtail(&mblock->mem_free_lst, &pmnode[i].free_lst_node);
        mptr = (aw_uchar_t *) mptr + size;
    }

    return mblock;
}

static aw_mem_block_t *
aw_mem_block_fetch(size_t size)
{
    aw_mem_block_t              *mblock;
    aw_mem_block_slot_tid_t     *slot_tid;
    aw_list_node_t              *block_lst_node;

    slot_tid = aw_mem_block_slot_tid_get(0, size);
    if (NULL == slot_tid) {

        return NULL;
    }

    if (aw_list_empty(&slot_tid->slot_block_lst)) {  //there is no block unbinding to a thread, create one

        mblock = aw_mem_block_create(0, size);
    } else {

        block_lst_node = aw_list_pophead(&slot_tid->slot_block_lst);
        mblock = aw_list_entry(block_lst_node, aw_mem_block_t, block_lst_node);
        --slot_tid->n_mem_block;
    }

    return mblock;
}

/*
 * typedef struct {
 *     aw_rbtree_node_t        slot_tid_rbt_node;  //link in aw_mem_block_slot_t.slot_tid_rbt
 *     aw_list_t               slot_block_lst;     //link block alloct to same thread
 *     aw_uint32_t             n_mem_block;
 * } aw_mem_block_slot_tid_t;
 */
static aw_mem_block_slot_tid_t *
aw_mem_block_slot_tid_create(aw_tid_t tid, size_t size)
{
    aw_mem_block_slot_tid_t        *slot_tid;
    aw_mem_block_t                 *mblock;
    aw_pool_t                      *manager_pool;
    aw_uint8_t                      slot_idx;
    aw_mem_block_slot_t            *slot;

    manager_pool = aw_mem_allocator->mem_manager_pool;

    slot_tid = aw_pool_calloc(manager_pool, sizeof(aw_mem_block_slot_tid_t));
    if (NULL == slot_tid) {

        return NULL;
    }

    if (0 == tid) {

        mblock = aw_mem_block_create(0, size);
        if (NULL == mblock) {

            aw_pool_free(manager_pool, slot_tid);
            return NULL;
        }
    } else {

        mblock = aw_mem_block_fetch(size);
        if (NULL == mblock) {

            aw_pool_free(manager_pool, slot_tid);
            return NULL;
        }
    }

    slot_idx = aw_get_slot_index(size);
    slot = &aw_mem_allocator->mem_block_slot[slot_idx];

    slot_tid->slot = slot;
    aw_rbtree_node_init(&slot->slot_tid_rbt, &slot_tid->slot_tid_rbt_node);
    slot_tid->slot_tid_rbt_node.key = tid;
    aw_rbtree_insert(&slot->slot_tid_rbt, &slot_tid->slot_tid_rbt_node);

    mblock->tid = tid;
    aw_list_init(&slot_tid->slot_block_lst);
    aw_list_pushtail(&slot_tid->slot_block_lst, &mblock->block_lst_node);
    slot_tid->n_mem_block = 1;
    mblock->slot_tid = slot_tid;

    return slot_tid;
}

/*
 * tyepdef struct {
 *     size_t                  slot_size;          //mem size to allocate in this slot
 *     aw_rbtree_t             slot_tid_rbt;       //block group by tid
 * } aw_mem_block_slot_t;
 */
static int
aw_mem_block_slot_init(aw_mem_block_slot_t *slot, size_t size)
{
    aw_mem_block_slot_tid_t     *slot_tid;

    slot->slot_size = size;
    aw_rbtree_init(&slot->slot_tid_rbt);

    slot_tid = aw_mem_block_slot_tid_create(0, size);
    if (NULL == slot_tid) {

        return AW_ERROR;
    } else {

        return AW_OK;
    }
}

static aw_mem_block_slot_tid_t *
aw_mem_block_slot_tid_get(aw_tid_t tid, size_t size)
{
    aw_mem_block_slot_tid_t        *slot_tid;
    aw_uint8_t                      slot_idx;
    aw_mem_block_slot_t            *slot;
    aw_rbtree_node_t               *slot_tid_rbt_node;

    slot_idx = aw_get_slot_index(size);
    slot = &aw_mem_allocator->mem_block_slot[slot_idx];

    slot_tid_rbt_node = aw_rbtree_search(&slot->slot_tid_rbt, tid);
    if (NULL == slot_tid_rbt_node) { //no free memory to use

        slot_tid = aw_mem_block_slot_tid_create(tid, size);
        if (NULL == slot_tid) {

            return NULL;
        }
    }
    slot_tid = aw_rbtree_entry(slot_tid_rbt_node, aw_mem_block_slot_tid_t, slot_tid_rbt_node);

    return slot_tid;
}

static aw_mem_block_t *
aw_mem_block_idle(aw_tid_t tid, size_t size)
{
    aw_mem_block_t                 *mblock;
    aw_mem_block_slot_tid_t        *slot_tid;
    aw_list_node_t                 *lst_node;

    slot_tid = aw_mem_block_slot_tid_get(tid, size);
    if (NULL == slot_tid) { //no free memory to use

        return NULL;
    }

    lst_node = aw_list_tail(&slot_tid->slot_block_lst);
    mblock = aw_list_entry(lst_node, aw_mem_block_t, block_lst_node);

    if (0 == mblock->n_mem_free) { //all block is full

        mblock = aw_mem_block_fetch(size);
        if (NULL == mblock) { //no free memory to use

            return NULL;
        }
        mblock->tid = tid;
        mblock->slot_tid = slot_tid;
        aw_list_pushtail(&slot_tid->slot_block_lst, &mblock->block_lst_node);
        ++slot_tid->n_mem_block;
    }

    return mblock;
}

static void *
aw_mem_block_malloc(size_t size)
{
    aw_mem_block_t                      *mblock;
    aw_tid_t                             tid;
    aw_mem_block_manager_node_t         *mnode;
    aw_list_node_t                      *lst_node;
    aw_mem_block_slot_tid_t             *slot_tid;

    tid = pthread_self(); //TODO 后续改为自己封装的线程库
    mblock = aw_mem_block_idle(tid, size);
    if (NULL == mblock) {

        return NULL;
    }

    lst_node = aw_list_pophead(&mblock->mem_free_lst);
    --mblock->n_mem_free;
    if (0 == mblock->n_mem_free) { //if block is full, push it into head of slot_tid->slot_block_lst

        slot_tid = mblock->slot_tid;
        aw_list_delete(&slot_tid->slot_block_lst, &mblock->block_lst_node);
        aw_list_pushhead(&slot_tid->slot_block_lst, &mblock->block_lst_node);
    }

    mnode = aw_list_entry(lst_node, aw_mem_block_manager_node_t,
                          free_lst_node);

    aw_rbtree_insert(&aw_mem_allocator->mem_inuse_rbt, &mnode->inuse_rbt_node);
    ++aw_mem_allocator->n_mem_inuse;

    return mnode->mptr;
}

static void
aw_mem_block_free(aw_rbtree_node_t *rbt_node)
{
    aw_mem_block_manager_node_t         *mnode;
    aw_mem_block_t                      *mblock;
    aw_mem_block_slot_tid_t             *slot_tid;

    mnode = aw_rbtree_entry(rbt_node, aw_mem_block_manager_node_t, inuse_rbt_node);
    aw_rbtree_delete(&aw_mem_allocator->mem_inuse_rbt, rbt_node);
    --aw_mem_allocator->n_mem_inuse;

    mblock = mnode->bptr;
    aw_list_pushtail(&mblock->mem_free_lst, &mnode->free_lst_node);
    if (0 == mblock->n_mem_free) {

        slot_tid = mblock->slot_tid;
        aw_list_delete(&slot_tid->slot_block_lst, &mblock->block_lst_node);
        aw_list_pushtail(&slot_tid->slot_block_lst, &mblock->block_lst_node);
    }
    ++mblock->n_mem_free;
}

/*
 * typedef struct {
 *     aw_rbtree_node_t        large_rbt_node;     //link to aw_mem_alloc_t.
 * } aw_mem_large_t;
 */
static void *
aw_mem_large_malloc(size_t size)
{
    aw_mem_large_t      *mlarge;
    size_t               mlarge_size;
    aw_pool_t           *mem_pool;
    void                *mptr;

    mem_pool = aw_mem_allocator->mem_pool;

    mlarge_size = sizeof(aw_mem_large_t) + size;
    mlarge = malloc(mlarge_size);
    if (NULL == mlarge) {

        return NULL;
    }
    mptr = (aw_uchar_t *) mlarge + sizeof(aw_mem_large_t);

    aw_rbtree_node_init(&aw_mem_allocator->mem_inuse_rbt, &mlarge->large_rbt_node);
    mlarge->large_rbt_node.key = aw_ptr_key(mptr);
    mlarge->large_rbt_node.usersigned = 1;
    aw_rbtree_insert(&aw_mem_allocator->mem_inuse_rbt, &mlarge->large_rbt_node);
    ++aw_mem_allocator->n_mem_inuse;

    return mptr;
}

static void
aw_mem_large_free(aw_rbtree_node_t *rbt_node)
{
    aw_mem_large_t      *mlarge;

    mlarge = aw_rbtree_entry(rbt_node, aw_mem_large_t, large_rbt_node);
    aw_rbtree_delete(&aw_mem_allocator->mem_inuse_rbt, rbt_node);
    --aw_mem_allocator->n_mem_inuse;

    free(mlarge);
}
