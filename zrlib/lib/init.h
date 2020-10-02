/**
 * @author zuri
 * @date mar. 29 sept. 2020 22:06:44 CEST
 */

#ifndef ZRLIB_INIT_H
#define ZRLIB_INIT_H

#include <zrlib/base/Identifier/Identifier.h>
#include <zrlib/base/struct.h>

void zrlib_initCurrentThread(void);
void zrlib_endCurrentThread(void);

ZRIdentifier* zrlib_getObjIdentifier(ZRObjInfos objInfos);
void zrlib_initObj(void *obj, ZRObjInfos objInfos);
void* zrlib_intern(void *obj, ZRObjInfos objInfos);

#define zrlib_initPType(obj) zrlib_initObj((obj), ZRTYPE_OBJINFOS(*(obj)))
#define zrlib_internPType(obj) zrlib_intern((obj), ZRTYPE_OBJINFOS(*(obj)))

#endif
