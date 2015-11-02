# AW Event Driver
---

The Project is a C event driver

It's UPDATING continuous

now I achieve key, list and hash, now makefile is not provided, You Can Compile and test these three module as following:

<p>gcc -o aa aw_key.c -I./ -I../include -DAW_UNIT_TEST -Wall -Wno-unused-parameter -Werror -g</p>
<p>gcc -o aa aw_list.c -I./ -I../include -DAW_UNIT_TEST -Wall -Wno-unused-parameter -Werror -g</p>
<p>gcc -o aa aw_rbtree.c -I./ -I../include -DAW_UNIT_TEST -Wall -Wno-unused-parameter -Werror -g</p>

Next will commit pool and mem allocator