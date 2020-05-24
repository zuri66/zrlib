/**
 * @author zuri
 * @date dim. 24 mai 2020 13:30:51 CEST
 */

#include <zrlib/base/Algorithm/fcmp.h>

#include <stdbool.h>
#include <stddef.h>

#define scmp(type) \
	long long diff = *(type*)a - *(type*)b; \
	\
	if (diff > 0) \
		return 1; \
	if (diff < 0) \
		return -1; \
	\
	return 0

int ZRFCmp_bool(void *a, void *b)
{
	return (int)*(bool*)a - (int)*(bool*)b;
}

int zrfcmp_char(void *a, void *b)
{
	return *(char*)a - *(char*)b;
}

int zrfcmp_short(void *a, void *b)
{
	return *(short*)a - *(short*)b;
}

int zrfcmp_int(void *a, void *b)
{
	return *(int*)a - *(int*)b;
}

int zrfcmp_long(void *a, void *b)
{
	scmp(long);
}

int zrfcmp_llong(void *a, void *b)
{
	scmp(long long);
}

#define ucmp(TYPE) \
	TYPE ua = *(TYPE*)a; \
	TYPE ub = *(TYPE*)b; \
	\
	if(ua > ub) \
		return 1; \
	if(ua < ub) \
		return -1; \
	\
	return 0

int zrfcmp_uchar(void *a, void *b)
{
	ucmp(unsigned char);
}

int zrfcmp_ushort(void *a, void *b)
{
	ucmp(unsigned short);
}

int zrfcmp_uint(void *a, void *b)
{
	ucmp(unsigned int);
}

int zrfcmp_ulong(void *a, void *b)
{
	ucmp(unsigned long);
}

int zrfcmp_ullong(void *a, void *b)
{
	ucmp(unsigned long long);
}

int zrfcmp_size_t(void *a, void *b)
{
	ucmp(size_t);
}

int zrfcmp_ptrdiff_t(void *a, void *b)
{
	scmp(ptrdiff_t);
}

int zrfcmp_ptrEq(void *a, void *b)
{
	return a == b;
}

#ifdef INTPTR_MAX
int zrfcmp_intptr(void *a, void *b)
{
	scmp(uintptr_t);
}
#endif

#ifdef UINTPTR_MAX
int zrfcmp_uintptr(void *a, void *b)
{
	ucmp(uintptr_t);
}
#endif
