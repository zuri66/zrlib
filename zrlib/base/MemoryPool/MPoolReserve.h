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

ZRObjInfos ZRMPoolReserveInfos_objInfos(void);

void ZRMPoolReserveInfos(void *infos, ZRObjInfos blockInfos, size_t nbBlocks);
void ZRMPoolReserveInfos_allocator(void *infos, ZRAllocator *allocator);
void ZRMPoolReserveInfos_mode(void *infos, enum ZRMPoolReserveModeE mode);
void ZRMPoolReserveInfos_areaMetaData(void *infos, ZRObjAlignInfos *areaMetaData);
void ZRMPoolReserveInfos_staticStrategy(void *infos);

ZRObjInfos ZRMPoolReserve_objInfos(void *infos);
void ZRMPoolReserve_init(ZRMemoryPool *pool, void *infos);
ZRMemoryPool* ZRMPoolReserve_new(void *infos);


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
