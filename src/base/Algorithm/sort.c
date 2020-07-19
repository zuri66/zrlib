/**
 * @author zuri
 * @date dim. 24 mai 2020 13:30:51 CEST
 */

#include <zrlib/base/Algorithm/sort.h>

#include <assert.h>

void ZRInsertionSort(void *array, size_t objSize, size_t nbObj, size_t nbAlreadySorted, zrfucmp fucmp, void *data)
{
	ZRINSERTIONSORT(array, objSize, nbObj, nbAlreadySorted, fucmp, data);
}
