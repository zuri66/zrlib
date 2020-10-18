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
	size_t capacity;
} ZRArrayAndCapacity;

typedef struct
{
	void *array;
	size_t nbObj;
} ZRArrayAndNbObj;

typedef struct
{
	void *array;
	ZRObjInfos objInfos;
	size_t capacity;
	size_t nbObj;
} ZRArray;


#define ZRARRAY2_DEF(A,S,SUFF) ((ZRArrayAnd ## SUFF) { (A), (S) })
#define ZRARRAYC_DEF(A,S) ZRARRAY2_DEF(A,S,Capacity)
#define ZRARRAYN_DEF(A,S) ZRARRAY2_DEF(A,S,NbObj)

/* offset, objSize, nbObj */
#define ZRARRAY_OON(A) (A).array, (A).objInfos.size, (A).nbObj
/* offset, objSize */
#define ZRARRAY_OO(A) (A).array, (A).objInfos.size
/* capacity, nbObj */
#define ZRARRAY_CN(A) (A).capacity, (A).nbObj

#endif
