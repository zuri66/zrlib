/**
 * @author zuri
 * @date ven. 04 sept. 2020 13:13:44 CEST
 */

#ifndef ZRARRAY_H
#define ZRARRAY_H

#include <zrlib/base/ArrayOp.h>
#include <zrlib/base/struct.h>

typedef struct
{
	void *array;
	size_t size;
} ZRArrayAndSize;

typedef struct
{
	void *array;
	size_t nbObj;
} ZRArrayAndNb;

typedef struct
{
	void *array;
	ZRObjInfos objInfos;
	size_t size;
	size_t nbObj;
} ZRArray;


#define ZRARRAY2_DEF(A,S,SUFF) ((ZRArrayAnd ## SUFF) { (A), (S) })
#define ZRARRAYS_DEF(A,S) ZRARRAY2_DEF(A,S,Size)
#define ZRARRAYN_DEF(A,S) ZRARRAY2_DEF(A,S,Nb)

/* offset, objSize, nbObj */
#define ZRARRAY_OON(A) (A).array, (A).objInfos.size, (A).nbObj
/* offset, objSize */
#define ZRARRAY_OO(A) (A).array, (A).objInfos.size
/* size, nbObj */
#define ZRARRAY_SN(A) (A).size, (A).nbObj

#endif
