/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:34:25 (UTC+0100)
 */

#ifndef MPOOLRESERVE_H
#define MPOOLRESERVE_H

#include <zrlib/config.h>

#include <zrlib/base/struct.h>
#include <zrlib/base/MemoryPool/MemoryPool.h>
#include <zrlib/base/Allocator/Allocator.h>

#include <stdbool.h>

// ============================================================================

// ============================================================================
// HELP
// ============================================================================

#define ZRMPoolReserveMode_default ZRMPoolReserveMode_list

enum ZRMPoolReserveModeE
{
	ZRMPoolReserveMode_bits,
	ZRMPoolReserveMode_list,
	ZRMPoolReserveMode_chunk,
};

ZRObjInfos ZRMPoolReserveIInfosObjInfos(void);

void ZRMPoolReserveIInfos(void *iinfo, ZRObjInfos blockInfos, size_t nbBlocks);
void ZRMPoolReserveIInfos_allocator(void *iinfo, ZRAllocator *allocator);
void ZRMPoolReserveIInfos_mode(void *iinfo, enum ZRMPoolReserveModeE mode);
void ZRMPoolReserveIInfos_areaMetaData(void *iinfo, ZRObjAlignInfos *areaMetaData);
void ZRMPoolReserveIInfos_staticStrategy(void *iinfo);

ZRObjInfos ZRMPoolReserve_objInfos(void *iinfo);
void ZRMPoolReserve_init(ZRMemoryPool *pool, void *iinfo);
ZRMemoryPool* ZRMPoolReserve_new(void *iinfo);


ZRMemoryPool* ZRMPoolReserve_createSimple(
	ZRObjInfos objInfos, size_t nbBlocks,
	ZRAllocator *allocator, enum ZRMPoolReserveModeE mode
	);

ZRMemoryPool* ZRMPoolReserve_create(
	ZRObjInfos objInfos, size_t nbBlocks,
	ZRObjAlignInfos *areaMetaData,
	ZRAllocator *allocator, enum ZRMPoolReserveModeE mode
	);

#endif
