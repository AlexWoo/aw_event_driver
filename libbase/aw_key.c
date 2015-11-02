#include <aw_core.h>

aw_key_t
aw_str_key(aw_uchar_t *str, size_t len)
{
    aw_uint32_t     key;
    int             i;

    key = 0;

    for (i = 0; i < len; ++i) {

        key = (aw_uint32_t)str[i] + key * 31;
    }

    return key;
}

aw_key_t
aw_strcase_key(aw_uchar_t *str, size_t len)
{
    aw_uint32_t     key;
    aw_uchar_t      lowcase;
    int             i;

    key = 0;

    for (i = 0; i < len; ++i) {

        lowcase = str[i] >= 'A' && str[i] <= 'Z'? str[i] | 0x20: str[i];
        key = (aw_uint32_t)lowcase + key * 31;
    }

    return key;
}

aw_key_t
aw_ptr_key(void *ptr)
{
    return (aw_key_t)ptr;
}

#ifdef AW_UNIT_TEST
int main()
{
    aw_key_t key, keycase, keylow, keyptr;
    int *pi;
    pi = malloc(sizeof(int));
    *pi = 10;

    key = aw_str_key((aw_uchar_t *)"Hello World!!", strlen("Hello World!!"));
    keycase = aw_strcase_key((aw_uchar_t *)"Hello World!!", strlen("Hello World!!"));
    keylow = aw_str_key((aw_uchar_t *)"hello world!!", strlen("hello world!!"));
    keyptr = aw_ptr_key(pi);

    printf("key: %u, keycase: %u, keylow: %u\n", key, keycase, keylow);
    printf("*pi: %d, pi: %p, keyptr: %x\n", *pi, pi, keyptr);

    return 1;
}
#endif
