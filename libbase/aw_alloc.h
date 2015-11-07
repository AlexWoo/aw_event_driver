#ifndef __AW_ALLOC_H__
#define __AW_ALLOC_H__

//TODO 内存对齐
//内存错误记录
//TODO mem_block_slot_tid 线程结束后复用
//TODO 多线程锁问题

void *aw_malloc(size_t size);
void *aw_calloc(size_t nmemb, size_t size);
void *aw_realloc(void *ptr, size_t size);
void  aw_free(void *ptr);

#endif
