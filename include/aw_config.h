#ifndef __AW_CONFIG_H__
#define __AW_CONFIG_H__

#include <aw_linux_config.h>

#ifndef AW_PAGESIZE
#define AW_PAGESIZE  4096
#endif

#ifndef AW_DEFAULT_POOL_SIZE
#define AW_DEFAULT_POOL_SIZE    AW_PAGESIZE
#endif

#ifndef AW_MEM_MANAGER_POOL_SIZE
#define AW_MEM_MANAGER_POOL_SIZE    AW_DEFAULT_POOL_SIZE
#endif

#ifndef AW_MEM_N_BLOCK_SLOT
#define AW_MEM_N_BLOCK_SLOT  8
#endif

#ifndef AW_MEM_BLOCK_FIRST_OFFSET
#define AW_MEM_BLOCK_FIRST_OFFSET    4
#endif

#ifndef AW_MEM_BLOCK_SIZE
#define AW_MEM_BLOCK_SIZE    (4 * AW_PAGESIZE)
#endif

#ifndef AW_MEM_POOL_SIZE
#define AW_MEM_POOL_SIZE    (1024 * AW_PAGESIZE)
#endif

#ifndef AW_DEFAULT_PTR_ALIGNMENT
#define AW_DEFAULT_PTR_ALIGNMENT   16
#endif

#endif
