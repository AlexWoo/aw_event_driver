CC = gcc
CFLAGS = -pipe -W -Wall -Wno-unused-parameter -Werror -g
LINK=$(CC)
TESTMACRO=

SRC_LIBBASE=		\
	./aw_list.c 	\
	./aw_key.c  	\
	./aw_rbtree.c 	\
	./aw_pool.c  	\
	./aw_alloc.c

OBJ_LIST=			\
	./aw_list.o

OBJ_KEY=			\
	./aw_key.o

OBJ_RBTREE=			\
	./aw_key.o 		\
	./aw_rbtree.o

OBJ_POOL=			\
	./aw_list.o 	\
	./aw_key.o 		\
	./aw_rbtree.o 	\
	./aw_pool.o

OBJ_ALLOC=			\
	./aw_list.o 	\
	./aw_key.o 		\
	./aw_rbtree.o 	\
	./aw_pool.o 	\
	./aw_alloc.o

DEPS=					\
	./aw_core.h 		\
	./aw_config.h 		\
	./aw_linux_config.h \

LIBBASE_DEPS=		\
	./aw_list.h 	\
	./aw_key.h 		\
	./aw_rbtree.h 	\
	./aw_pool.h 	\
	./aw_alloc.h

INCL=				\
	-I ./			\
	-I ../include


help:
	@echo "make testlist"
	@echo "make testkey"
	@echo "make testrbtree"
	@echo "make testpool"
	@echo "make testalloc"

clean:
	@rm -rf *.o
	@rm -rf ./list ./key ./rbtree ./pool ./alloc

testlist:
	@$(MAKE) clean
	@$(MAKE) TESTMACRO=-DAW_LIST_TEST list

testkey:
	@$(MAKE) clean
	@$(MAKE) TESTMACRO=-DAW_KEY_TEST key

testrbtree:
	@$(MAKE) clean
	@$(MAKE) TESTMACRO=-DAW_RBTREE_TEST rbtree

testpool:
	@$(MAKE) clean
	@$(MAKE) TESTMACRO=-DAW_POOL_TEST pool

testalloc:
	@$(MAKE) clean
	@$(MAKE) TESTMACRO=-DAW_ALLOC_TEST alloc





list: $(OBJ_LIST)
	$(LINK) -o ./list $(OBJ_LIST)

key: $(OBJ_KEY)
	$(LINK) -o ./key $(OBJ_KEY)

rbtree: $(OBJ_RBTREE)
	$(LINK) -o ./rbtree $(OBJ_RBTREE)

pool: $(OBJ_POOL)
	$(LINK) -o ./pool $(OBJ_POOL)

alloc: $(OBJ_ALLOC)
	$(LINK) -o ./alloc $(OBJ_ALLOC)

./aw_list.o: ./aw_list.c
	$(CC) -c $(CFLAGS) $(INCL) $(TESTMACRO) -o ./aw_list.o ./aw_list.c

./aw_key.o: ./aw_key.c
	$(CC) -c $(CFLAGS) $(INCL) $(TESTMACRO) -o ./aw_key.o ./aw_key.c

./aw_rbtree.o: ./aw_rbtree.c
	$(CC) -c $(CFLAGS) $(INCL) $(TESTMACRO) -o ./aw_rbtree.o ./aw_rbtree.c

./aw_pool.o: ./aw_pool.c
	$(CC) -c $(CFLAGS) $(INCL) $(TESTMACRO) -o ./aw_pool.o ./aw_pool.c

./aw_alloc.o: ./aw_alloc.c
	$(CC) -c $(CFLAGS) $(INCL) $(TESTMACRO) -o ./aw_alloc.o ./aw_alloc.c

