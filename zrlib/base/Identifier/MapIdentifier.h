/**
 * @author zuri
 * @date mercredi 13 novembre 2019, 18:53:08 (UTC+0100)
 */

#ifndef ZRMAPIDENTIFIER_H
#define ZRMAPIDENTIFIER_H

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Map/Map.h>
#include <zrlib/base/Identifier/Identifier.h>
#include <zrlib/base/struct.h>


ZRObjInfos ZRMapIdentifierIInfosObjInfos(void);

void ZRMapIdentifierIInfos(void *iinfos, ZRObjInfos objInfos, zrfuhash *fuhash, size_t nbfhash);
void ZRMapIdentifierIInfos_staticStrategy(void *iinfos);
void ZRMapIdentifierIInfos_allocator(void *iinfos, ZRAllocator *allocator);
void ZRMapIdentifierIInfos_fucmp(void *iinfos, zrfucmp fucmp);

ZRObjInfos ZRMapIdentifier_objInfos(void *iinfos);
void ZRMapIdentifier_init(ZRIdentifier *identifier, void *iinfos);
ZRIdentifier* ZRMapIdentifier_new(void *iinfos);

#endif
