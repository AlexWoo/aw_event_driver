#ifndef __AW_KEY_H__
#define __AW_KEY_H__

#include <aw_core.h>

aw_key_t aw_str_key(aw_uchar_t *str, size_t len);
aw_key_t aw_strcase_key(aw_uchar_t *str, size_t len);

aw_key_t aw_ptr_key(void *ptr);

#endif
