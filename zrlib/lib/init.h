/**
 * @author zuri
 * @date mar. 29 sept. 2020 22:06:44 CEST
 */

#ifndef ZRLIB_INIT_H
#define ZRLIB_INIT_H

#include <zrlib/base/Identifier/Identifier.h>
#include <zrlib/base/struct.h>

#define ZRSERVICE_XLIST() \
	X(allocator, "allocator")

typedef struct
{
	ZRID id;
	char *name;
}ZRServiceID;

#define X(NAME,STR) ZRService_ ## NAME,
typedef enum{
	ZRSERVICE_XLIST()
	ZRSERVICES_NB,
}ZRServices;
#undef X

#define ZRSERVICE_ID(S) ZRSERVICESID[S].id
#define ZRSERVICE_NAME(S) ZRSERVICESID[S].name

extern ZRServiceID ZRSERVICESID[ZRSERVICES_NB];

void zrlib_initCurrentThread(void);
void zrlib_endCurrentThread(void);

ZRIdentifier* zrlib_getObjIdentifier(ZRObjInfos objInfos);
void zrlib_initObj(void *obj, ZRObjInfos objInfos);
void* zrlib_intern(void *obj, ZRObjInfos objInfos);

ZRObjectP zrlib_getService(char *name);
ZRObjectP zrlib_getServiceFromID(ZRID id);
ZRID zrlib_registerService(char *name, ZRObjectP *object);

#define zrlib_initPType(obj) zrlib_initObj((obj), ZRTYPE_OBJINFOS(*(obj)))
#define zrlib_internPType(obj) zrlib_intern((obj), ZRTYPE_OBJINFOS(*(obj)))

#endif
