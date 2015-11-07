# AW Event Driver
---

## Basic Instructions

The Project is a C event driver, system will be driven by event like request from socket, system signal, timer. User can load Module in the system as dynamic library

## BASE LIB
### list
A Circular List with two direction struct. You can put the listnode into your struct, and use **aw\_list\_entry** to find your struct instance ptr through listnode address

You can get more information to from the test code in aw_list.c

### key
Covert string or point to aw_key_t which is actually unsigned long type

You can get more information to from the test code in aw_key.c

### rbtree

A Red Black Tree struct. You can put the rbtreenode into your struct, and use **aw\_rbtree\_entry** to find your struct instance ptr through rbtreenode address

You can get more information to from the test code in aw_rbtree.c

### pool

A memory pool. You can allocator memory from the pool, if you don't need to release these memory or you want to reuse these memory. You can destroy a pool to clear all the data in pool once for all

### alloc

It's a memory allocator and not finish yet, I will write another instructions about this memory allocator when I finish this module

## TODO LIST

It's UPDATING continuous

for test

	make testlist
	make testkey
	make testrbtree
	make testpool
	make testalloc

mem allocator is not tested, TODO:

	memory align for mem_block and mem_large
	record memory error
	add thread lock to support multithread
	a thread lib work with mem allocator


