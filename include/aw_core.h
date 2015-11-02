#ifndef __AW_CORE_H__
#define __AW_CORE_H__

#include <aw_config.h>

#define AW_OK        0
#define AW_ERROR    -1

typedef unsigned char               aw_uchar_t;

typedef unsigned char               aw_uint8_t;
typedef unsigned short              aw_uint16_t;
typedef unsigned                    aw_uint32_t;
typedef unsigned long long          aw_uint64_t;

typedef unsigned long               aw_key_t;
typedef pthread_t                   aw_tid_t;

typedef struct aw_list_node_s       aw_list_node_t;
typedef struct aw_list_s            aw_list_t;

typedef struct aw_rbtree_node_s     aw_rbtree_node_t;
typedef struct aw_rbtree_s          aw_rbtree_t;

#include <aw_key.h>
#include <aw_list.h>
#include <aw_rbtree.h>
#include <aw_pool.h>
#include <aw_alloc.h>

#endif
