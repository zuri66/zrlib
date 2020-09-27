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


ZRObjInfos ZRMapIdentifierInfos_objInfos(void);
ZRObjInfos ZRMapIdentifier_objInfos(void *infos);

void ZRMapIdentifierInfos(void *infos_out, ZRObjInfos objInfos, zrfuhash *fuhash, size_t nbfhash, ZRAllocator *allocator);
void ZRMapIdentifierInfos_done(void *infos_out);

void ZRMapIdentifier_init(ZRIdentifier *identifier, void *infos);
ZRIdentifier* ZRMapIdentifier_new(void *infos);

#endif
