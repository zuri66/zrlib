/**
 * @author zuri
 * @date dim. 24 mai 2020 13:30:51 CEST
 */

#include <zrlib/config.h>

#include <stdint.h>

int ZRFCmp_bool(void *a, void *b);

int zrfcmp_char(void *a, void *b);
int zrfcmp_short(void *a, void *b);
int zrfcmp_int(void *a, void *b);
int zrfcmp_long(void *a, void *b);
int zrfcmp_llong(void *a, void *b);

int zrfcmp_uchar(void *a, void *b);
int zrfcmp_ushort(void *a, void *b);
int zrfcmp_uint(void *a, void *b);
int zrfcmp_ulong(void *a, void *b);
int zrfcmp_ullong(void *a, void *b);

int zrfcmp_size_t(void *a, void *b);
int zrfcmp_ptrdiff_t(void *a, void *b);
int zrfcmp_ptrEq(void *a, void *b);

#ifdef INTPTR_MAX
int zrfcmp_intptr(void *a, void *b);
#endif

#ifdef UINTPTR_MAX
int zrfcmp_uintptr(void *a, void *b);
#endif
