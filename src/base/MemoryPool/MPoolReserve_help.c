/**
 * @author zuri
 * @date mardi 26 novembre 2019, 00:23:51 (UTC+0100)
 */

ZRMemoryPool* ZRMPoolReserve_create(
	size_t blockSize, size_t alignment, size_t nbBlocks,
	ZRObjAlignInfos *areaMetaData,
	ZRAllocator *allocator,
	enum ZRMPoolReserveModeE mode
	)
{
	ZRObjAlignInfos infos[ZRMAX(ZRMPOOLRLIST_INFOS_NB, ZRMPOOLRBITS_INFOS_NB, ZRMPOOLRCHUNK_INFOS_NB)];
	ZRObjAlignInfos areaHeadInfos[ZRAREAHEADINFOS_NB];

	ZRAreaHeadInfos_make(areaHeadInfos, areaMetaData);

	ZRMemoryPool *pool;
	ZRMPoolInfos *pinfos;
	ZRMemoryPoolStrategy *strategy = ZRALLOC(allocator, sizeof(ZRMPoolReserveStrategy));
	size_t const headSize =  areaHeadInfos[ZRAreaHeadInfos_struct].size + ZRAREA_HEAD_GUARD_SIZE;
	size_t const nbBlocksForAreaHead = headSize / blockSize + (headSize % blockSize ? 1 : 0);

	// TODO: better strategy for area head ?
	size_t const nbAreaInSpace = nbBlocks;
	nbBlocks += nbBlocksForAreaHead * nbAreaInSpace;

	ZRMPoolReserveStrategy_init(strategy, allocator, mode);

	switch (mode)
	{
	case ZRMPoolReserveMode_bits:
		{
		size_t const nbZRBits = nbBlocks / ZRBITS_NBOF + ((nbBlocks % ZRBITS_NBOF) ? 1 : 0);
		MPoolRBitsInfos(infos, blockSize, alignment, nbBlocks, nbZRBits);

		size_t const poolSize = infos[ZRMPoolRBitsInfos_struct].size;
		ZRMPoolRBits *rbpool = ZRALLOC(allocator, poolSize);
		pinfos = &rbpool->infos;

		rbpool->bits = (ZRBits*)((char*)rbpool + infos[ZRMPoolRBitsInfos_bits].offset);
		pinfos->reserve = ((char*)rbpool + infos[ZRMPoolRBitsInfos_reserve].offset);
		rbpool->nbZRBits = nbZRBits;
		pool = &rbpool->pool;
		break;
	}
	case ZRMPoolReserveMode_list:
		{
		MPoolRListInfos(infos, blockSize, alignment, nbBlocks);

		size_t const poolSize = infos[ZRMPoolRListInfos_struct].size;
		ZRMPoolRList *rlpool = ZRALLOC(allocator, poolSize);
		pinfos = &rlpool->infos;

		rlpool->nextUnused = (ZRReserveNextUnused*)((char*)rlpool + infos[ZRMPoolRListInfos_nextUnused].offset);
		pinfos->reserve = ((char*)rlpool + infos[ZRMPoolRListInfos_reserve].offset);
		pool = &rlpool->pool;
		break;
	}

	case ZRMPoolReserveMode_chunk:
		{
		size_t const nbChunks = nbBlocks / 2 + 1;
		MPoolRChunkInfos(infos, blockSize, alignment, nbBlocks, nbChunks);

		size_t const poolSize = infos[ZRMPoolRChunkInfos_struct].size;
		ZRMPoolRChunk *rcpool = ZRALLOC(allocator, poolSize);
		pinfos = &rcpool->infos;

		rcpool->nbChunks = nbChunks;
		rcpool->chunks = (ZRReserveMemoryChunk*)((char*)rcpool + infos[ZRMPoolRChunkInfos_chunk].offset);
		pinfos->reserve = ((char*)rcpool + infos[ZRMPoolRChunkInfos_reserve].offset);
		pool = &rcpool->pool;
		break;
	}
	}

	if (areaMetaData)
	{
		pinfos->areaHeadMetaDataOffset = areaMetaData->offset;
		pinfos->areaHeadMetaDataSize = areaMetaData->size;
	}
	else
	{
		pinfos->areaHeadMetaDataOffset = 0;
		pinfos->areaHeadMetaDataSize = 0;
	}
	pinfos->areaHeadSize = areaHeadInfos[ZRAreaHeadInfos_struct].size + ZRAREA_HEAD_GUARD_SIZE;
	pinfos->nbBlocksForAreaHead = nbBlocksForAreaHead;
	pinfos->nbBlocksTotal = nbBlocks;
	pinfos->nbAvailables = nbBlocks;

	strategy->fdestroy = ZRMPoolReserve_destroy;
	ZRMPOOL_INIT(pool, blockSize, strategy);
	return pool;
}

ZRMemoryPool* ZRMPoolReserve_createSimple(
	size_t blockSize, size_t alignment, size_t nbBlocks,
	ZRAllocator *allocator, enum ZRMPoolReserveModeE mode
	)
{
	return ZRMPoolReserve_create(blockSize, alignment, nbBlocks, NULL, allocator, mode);
}

void ZRMPoolReserve_destroy(ZRMemoryPool *pool)
{
	ZRAllocator *allocator = ZRMPOOL_STRATEGY(pool)->allocator;
	ZRMPOOL_DONE(pool);
	ZRFREE(allocator, pool->strategy);
	ZRFREE(allocator, pool);
}
