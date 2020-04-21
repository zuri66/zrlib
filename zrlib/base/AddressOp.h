/*
 * @author zuri
 * @date mardi 21 avril 2020, 12:37:58 (UTC+0200)
 */

#ifndef ZRADDRESSOP_H
#define ZRADDRESSOP_H

#include <zrlib/base/macro.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * Number of bits to use for intrinsics in size_t.
 */
#if SIZE_MAX <= 4294967295U
#	define ZRSIZE_NBBITS 32
#elif SIZE_MAX <= 18446744073709551615UL
#	define ZRSIZE_NBBITS 64
#else
#endif

#if (__has_include(<x86intrin.h>))
#include <x86intrin.h>


#if ZRSIZE_NBBITS <= 32 || ZRSIZE_NBBITS <= 64
#	define ZRADDRESS_I

#	define ZRSIZE_MASK_1R ((size_t)1)
#	define ZRSIZE_MASK_1L ~(ZRSIZE_MASK_FULL >> 1)
#	define ZRSIZE_MASK_FULL (SIZE_MAX)

// ============================================================================
// LOCAL MACROS

#	define FACTOR_NBOFBITS(pref,suff) ZRCONCAT(pref, ZRCONCAT(ZRSIZE_NBBITS,suff))
#	define SUFFIX_NBOFBITS(suff) ZRCONCAT(suff,ZRSIZE_NBBITS)

#	define _lzcnt_u SUFFIX_NBOFBITS(_lzcnt_u)
#endif

#endif //x86intrin_h


#ifdef ZRADDRESS_I
#	define ZRAddressOp_upPow2 ZRAddressOp_upPow2_i
#else
#ifdef ZRSIZE_NBBITS
#	define ZRAddressOp_upPow2 ZRAddressOp_upPow2_size_t
#else
#	error __FILE__ ": Cannot define the library."
#endif
#endif


#ifdef ZRADDRESS_I

static inline size_t ZRAddressOp_upPow2_i(size_t i)
{
	if(i <= 2) return i;
	return ZRSIZE_MASK_1L >> (_lzcnt_u(i - 1) - 1);
}
#endif


#ifdef ZRSIZE_NBBITS

static inline size_t ZRAddressOp_upPow2_size_t(size_t i)
{
	i--;
	i |= i >> 1;
	i |= i >> 2;
	i |= i >> 4;
	i |= i >> 8;
	i |= i >> 16;
#	if ZRSIZE_NBBITS > 32
	i |= i >> 32;
#	endif
	i++;
	return i;
}
#endif

//TODO: ifdef ZRFLAT
#define ZRAddress_inside ZRAddress_inArray


static inline bool ZRAddress_inside_std(void *offset, void *array, size_t objSize, size_t nb)
{
	size_t i;

	for(i = 0 ; i < nb ; i++)
	{
		if(offset == array)
			return true;

		array = (char*)array + objSize;
	}
	return false;
}

static inline bool ZRAddress_inArray(void *offset, void *array, size_t objSize, size_t nb)
{
	uintptr_t o = (uintptr_t)offset;
	uintptr_t a = (uintptr_t)array;
	return a <= o && o <= a + (objSize * nb);
}

// =================================================================================

static inline bool ZRAddress_aligned_a(void *offset, size_t alignment)
{
	return (uintptr_t)offset % alignment == 0;
}

static inline size_t ZRAddressOp_alignUp_a(void *offset, size_t alignment)
{
	size_t const rest = (uintptr_t)offset % alignment;

	if (rest)
		return alignment - rest;

	return 0;
}

static inline size_t ZRAddressOp_alignDown_a(void *offset, size_t alignment)
{
	return (uintptr_t)offset % alignment;
}

// =================================================================================
/*
 * The following is valid for 2 complemented integer representation.
 */

static inline bool ZRAddress_aligned_pow2(void *offset, size_t alignment)
{
	assert(ZRISPOW2SAFE(alignment));
	return (uintptr_t)offset & (alignment - 1) == 0;
}

static inline size_t ZRAddressOp_alignUp_pow2(void *offset, size_t alignment)
{
	assert(ZRISPOW2SAFE(alignment));
	return (-(uintptr_t)offset) & (alignment - 1);
}

static inline size_t ZRAddressOp_alignDown_pow2(void *offset, size_t alignment)
{
	assert(ZRISPOW2SAFE(alignment));
	return ((uintptr_t)offset) & (alignment - 1);
}

#endif /* ZRADDRESSOP_H */
