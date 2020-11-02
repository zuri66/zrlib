/**
 * @author zuri
 * @date jeu. 08 oct. 2020 19:39:16 CEST
 */

#ifndef ZRIDGENERATOR_H
#define ZRIDGENERATOR_H

#include <zrlib/lib/init.h>
#include <zrlib/base/Allocator/Allocator.h>

typedef struct
{
	size_t nbGenerated;
}ZRIDGenerator;

/* ========================================================================= */

ZRMUSTINLINE
static inline size_t ZRIDGENERATOR_NBGENERATED(ZRIDGenerator *generator)
{
	return generator->nbGenerated;
}

/* ========================================================================= */

size_t ZRIDGenerator_nbGenerated(ZRIDGenerator *generator);

ZRID ZRIDGenerator_nextID(ZRIDGenerator *generator);
ZRID ZRIDGenerator_generate(ZRIDGenerator *generator);
void ZRIDGenerator_release(ZRIDGenerator *generator, ZRID id);
void ZRIDGenerator_releaseAll(ZRIDGenerator *generator);
bool ZRIDGenerator_present(ZRIDGenerator *generator, ZRID id);

void ZRIDGenerator_destroy(ZRIDGenerator *generator);


ZRObjInfos ZRIDGeneratorIInfosObjInfos(void);
void ZRIDGeneratorIInfos(void *iinfos);
void ZRIDGeneratorIInfos_staticStrategy(void *iinfos);
void ZRIDGeneratorIInfos_allocator(void *iinfos, ZRAllocator *allocator);

ZRObjInfos ZRIDGenerator_objInfos(void *iinfos);
void ZRIDGenerator_init(ZRIDGenerator *generator, void *iinfos);
ZRIDGenerator* ZRIDGenerator_new(void *iinfos);

#endif
