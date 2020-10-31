/**
 * @author zuri
 * @date dimanche 20 janvier 2019, 18:08:52 (UTC+0100)
 */

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <zrlib/config.h>
#include <zrlib/base/struct.h>

#include <stddef.h>

// ============================================================================

typedef struct ZRAllocatorS ZRAllocator;

struct ZRAllocatorS
{
	void* (*falloc)__(ZRAllocator *allocator, size_t nbBytes);
	void* (*faalloc)_(ZRAllocator *allocator, size_t alignment, size_t nbBytes);
	void* (*frealloc)(ZRAllocator *allocator, void *allocated, size_t nbBytes);
	void (*ffree)(ZRAllocator *allocator, void *allocated);
};

// ============================================================================

ZRMUSTINLINE
static inline void* ZRALLOC(ZRAllocator *allocator, size_t nbBytes)
{
	return allocator->falloc(allocator, nbBytes);
}

ZRMUSTINLINE
static inline void* ZRAALLOC(ZRAllocator *allocator, size_t alignment, size_t nbBytes)
{
	return allocator->faalloc(allocator, alignment, nbBytes);
}

ZRMUSTINLINE
static inline void* ZROBJALLOC(ZRAllocator *allocator, ZRObjInfos objInfos)
{
	return allocator->faalloc(allocator, ZROBJINFOS_ALIGNMENT_SIZE(objInfos));
}

ZRMUSTINLINE
static inline void* ZRREALLOC(ZRAllocator *allocator, void *allocated, size_t nbBytes)
{
	return allocator->frealloc(allocator, allocated, nbBytes);
}

ZRMUSTINLINE
static inline void ZRFREE(ZRAllocator *allocator, void *allocated)
{
	allocator->ffree(allocator, allocated);
}

#define _ZRALLOC_VARIABLE(POS,V) V = ZRARRAYOP_GET(mem,1,infos[POS].offset);

/*
 * Alloc a unique memory object able to store multiple variables given in parameter.
 *
 * Each parameter must be a pair where the first argument is a void pointer and the second an ObjInfos representing the object pointed.
 * Each pointer is set to the place which can store its object.
 * The first variable point to the memory and must be free after use.
 */
#define ZRALLOC_VARIABLES(allocator, ...) ZRBLOCK( \
	ZRObjAlignInfos infos[] = { ZRARGS_XODD(ZROBJINFOS_CPYOBJALIGNINFOS, __VA_ARGS__), ZROBJALIGNINFOS_DEF0() }; \
	ZRStruct_bestOffsetsPos(ZRCARRAY_NBOBJ(infos) - 1, infos, 1); \
	void *mem = ZROBJALLOC(allocator, ZROBJALIGNINFOS_CPYOBJINFOS(infos[ZRCARRAY_NBOBJ(infos) - 1])); \
	ZRARGS_XCAPPLY(_ZRALLOC_VARIABLE, ZRARGS_EVEN(__VA_ARGS__)) \
	)

#define ZRALLOC_VARIABLES_E(allocator, ...) ZREVAL(ZRALLOC_VARIABLES(allocator, __VA_ARGS__))

/**
 * Same as ZRALLOC_VARIABLES but arguments are passed as an array of ZRObjetP where object pointed must be of void** type.
 */
ZRMUSTINLINE
static inline void ZRALLOC_OBJECTS(ZRAllocator *allocator, ZRObjectP *objects, size_t nb)
{
	size_t const nbInfos = nb + 1;
	ZRObjAlignInfos infos[nbInfos];

#define CPY(O) ZROBJINFOS_CPYOBJALIGNINFOS(O.infos)
	ZRCARRAY_XCPY(infos, objects, nb, CPY);
#undef CPY
	infos[nb] = ZROBJALIGNINFOS_DEF0();

	ZRStruct_bestOffsetsPos(nb, infos, 1);

	void *mem = ZROBJALLOC(allocator, ZROBJALIGNINFOS_CPYOBJINFOS(infos[nb]));

	for (size_t i = 0; i < nb - 1; i++)
		*(void**)objects[i].object = ZRARRAYOP_GET(mem, 1, infos[i].offset);

#define CPY(O) ZROBJALIGNINFOS_CPYOBJINFOS(O)
#define SET(D,S) D.infos = S
	ZRCARRAY_XXCPY(objects, infos, nb, CPY, SET);
#undef CPY
#undef SET
}

#endif
