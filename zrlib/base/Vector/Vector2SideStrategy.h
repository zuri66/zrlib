/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#ifndef VECTOR2SIDESTRATEGY
#define VECTOR2SIDESTRATEGY

#include <zrlib/config.h>
#include "./Vector.h"

typedef struct
{
	ZRVectorStrategy strategy;
} ZRVector_2SideStrategy;

// ============================================================================

ZRVector_2SideStrategy ZRVector_2SideStrategy_init();

void ZRVector_2SideStrategy_insert(ZRVector *vec, size_t pos);
void ZRVector_2SideStrategy_delete(ZRVector *vec, size_t pos);

#endif
