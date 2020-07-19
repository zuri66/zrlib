/**
 * @author zuri
 * @date dim. 24 mai 2020 13:30:51 CEST
 */

#include <zrlib/base/ArrayOp.h>

#include <assert.h>
#include <string.h>

/**
 * Classic insertion sort using ZRArrayOp_binsert_pos for finding the location of the element.
 *
 * The algorithm is not guarantee to be stable because of binsert_pos.
 */
ZRMUSTINLINE
static inline void ZRINSERTIONSORT(void *array, size_t objSize, size_t nbObj, size_t nbAlreadySorted, zrfucmp fucmp, void *data)
{
	assert(nbAlreadySorted <= nbObj);
	char obj[objSize];

	for (; nbAlreadySorted < nbObj; nbAlreadySorted++)
	{
		void *const item = ZRARRAYOP_GET(array, objSize, nbAlreadySorted);
		size_t const pos = ZRARRAYOP_BINSERT_POS(item, objSize, nbAlreadySorted, item, fucmp, data);
		void *const place = ZRARRAYOP_GET(array, objSize, pos);
		memcpy(obj, item, objSize);
		ZRARRAYOP_CPY(place, objSize, nbAlreadySorted - pos, ZRARRAYOP_GET(array,objSize, pos+1));
		memcpy(place, obj, objSize);
	}
}
