/**
 * @author zuri
 * @date mardi 26 novembre 2019, 00:23:51 (UTC+0100)
 */

ZRMemoryPool* ZRMPoolReserve_create(size_t blockSize, size_t alignment, size_t nbBlocks, ZRAllocator *allocator, bool bitStrategy)
{
	ZRObjAlignInfos infos[ZRMPOOLRLIST_INFOS_NB];
	ZRMemoryPool *pool;
	ZRMPoolInfos *pinfos;
	ZRMemoryPoolStrategy *strategy = ZRALLOC(allocator, sizeof(ZRMPoolReserveStrategy));
	size_t const nbBlocksForAreaHead = ZRAREA_HEAD_SIZE / blockSize + (ZRAREA_HEAD_SIZE % blockSize ? 1 : 0);

	// TODO: better strategy for area head ?
	size_t const nbAreaInSpace = nbBlocks;
	nbBlocks += nbBlocksForAreaHead * nbAreaInSpace;

	if (bitStrategy)
	{
		size_t const nbZRBits = ZRBITS_NBOF / nbBlocks + (ZRBITS_NBOF % nbBlocks) ? 1 : 0;
		MPoolRBitsInfos(infos, blockSize, alignment, nbBlocks, nbZRBits);

		ZRMPoolReserve_init(strategy, allocator, true);

		size_t const poolSize = infos[ZRMPoolRBitsInfos_struct].size;
		ZRMPoolRBits *rbpool = ZRALLOC(allocator, poolSize);
		pinfos = &rbpool->infos;

		rbpool->bits = (ZRBits*)((char*)rbpool + infos[ZRMPoolRBitsInfos_bits].offset);
		pinfos->reserve = ((char*)rbpool + infos[ZRMPoolRBitsInfos_reserve].offset);
		rbpool->nbZRBits = nbZRBits;
		pool = &rbpool->pool;
	}
	else
	{
		MPoolRListInfos(infos, blockSize, alignment, nbBlocks);
		ZRMPoolReserve_init(strategy, allocator, false);

		size_t const poolSize = infos[ZRMPoolRListInfos_struct].size;
		ZRMPoolRList *rlpool = ZRALLOC(allocator, poolSize);
		pinfos = &rlpool->infos;

		rlpool->nextUnused = (ZRReserveNextUnused*)((char*)rlpool + infos[ZRMPoolRListInfos_nextUnused].offset);
		pinfos->reserve = ((char*)rlpool + infos[ZRMPoolRListInfos_reserve].offset);
		pool = &rlpool->pool;
	}
	pinfos->nbBlocksForAreaHead = nbBlocksForAreaHead;
	pinfos->nbBlocksTotal = nbBlocks;
	pinfos->nbAvailables = nbBlocks;

	strategy->fdestroy = ZRMPoolReserve_destroy;
	ZRMPOOL_INIT(pool, blockSize, strategy);
	return pool;
}

void ZRMPoolReserve_destroy(ZRMemoryPool *pool)
{
	ZRAllocator *allocator = ZRMPOOL_STRATEGY(pool)->allocator;
	ZRMPOOL_DONE(pool);
	ZRFREE(allocator, pool->strategy);
	ZRFREE(allocator, pool);
}
