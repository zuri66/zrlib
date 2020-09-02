/**
 * @author zuri
 * @date mer. 02 sept. 2020 15:48:02 CEST
 */

#include <zrlib/config.h>
#include <zrlib/base/Allocator/Allocator.h>

#include <stdbool.h>

typedef bool (*zrfmustGrow)(size_t totalSpace, size_t usedSpace, void *userData);
typedef size_t (*zrfincreaseSpace)(size_t totalSpace, size_t usedSpace, void *userData);

typedef bool (*zrfmustShrink)(size_t totalSpace, size_t usedSpace, void *userData);
typedef size_t (*zrfdecreaseSpace)(size_t totalSpace, size_t usedSpace, void *userData);

static inline size_t ZRRESIZE_MORESIZE(
	size_t totalSpace, size_t usedSpace,
	zrfmustGrow fmustGrow, zrfincreaseSpace fincreaseSpace,
	void *userData
	)
{
	size_t nextTotalSpace = totalSpace;

	while (nextTotalSpace < usedSpace)
		nextTotalSpace = fincreaseSpace(nextTotalSpace, usedSpace, userData);
	while (fmustGrow(nextTotalSpace, usedSpace, userData))
		nextTotalSpace = fincreaseSpace(nextTotalSpace, usedSpace, userData);

	return nextTotalSpace;
}

static inline size_t ZRRESIZE_LESSSIZE(
	size_t totalSpace, size_t usedSpace,
	zrfmustGrow fmustShrink, zrfincreaseSpace fdecreaseSpace,
	void *userData
	)
{
	size_t nextTotalSpace = totalSpace;

	while (fmustShrink(nextTotalSpace, usedSpace, userData))
		nextTotalSpace = fdecreaseSpace(nextTotalSpace, usedSpace, userData);

	return nextTotalSpace;
}
