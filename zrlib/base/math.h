/**
 * @author zuri
 * @date mardi 18 décembre 2018, 22:46:00 (UTC+0100)
 */

#ifndef ZRMATH_H
#define ZRMATH_H

#include <zrlib/base/MemoryOp.h>

#include <assert.h>

#define ZRORDER2(A,B) ZRBLOCK( \
	assert(sizeof(A) == sizeof(B)); \
	if(A > B) ZRMemoryOp_swap(&A, &B, sizeof(A)); \
	)

ZRMUSTINLINE
static inline size_t ZRMATH_GCD(size_t a, size_t b)
{
	if (a == b)
		return a;
	if (a == 0)
		return b;
	if (b == 0)
		return a;

	size_t shift = 0;

	while (((a | b) & 1) == 0)
		a >>= 1, b >>= 1, shift++;
	while ((a & 1) == 0)
		a >>= 1;
	do
	{
		while ((b & 1) == 0)
			b >>= 1;

		if (a > b)
			ZRSWAP(size_t, a, b);

		b -= a;
	} while (b != 0);

	return a << shift;
}

#endif
