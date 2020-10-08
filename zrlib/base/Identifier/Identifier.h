/**
 * @author zuri
 * @date mercredi 13 novembre 2019, 18:53:08 (UTC+0100)
 */

#ifndef ZRIDENTIFIER_H
#define ZRIDENTIFIER_H

#include <zrlib/config.h>
#include <stdbool.h>

// ============================================================================

typedef struct ZRIdentifierStrategyS ZRIdentifierStrategy;
typedef struct ZRIdentifierS ZRIdentifier;

// ============================================================================

struct ZRIdentifierStrategyS
{
	ZRID (*fgetID)(ZRIdentifier *identifier, void *obj);
	void* (*fintern)(ZRIdentifier *identifier, void *obj);
	void* (*ffromID)(ZRIdentifier *identifier, ZRID id);

	bool (*fcontains)(ZRIdentifier *identifier, void *obj);
	bool (*frelease)(ZRIdentifier *identifier, void *obj);
	bool (*freleaseID)(ZRIdentifier *identifier, ZRID id);
	bool (*freleaseAll)(ZRIdentifier *identifier);

	void (*fdone)(ZRIdentifier *identifier);
	void (*fdestroy)(ZRIdentifier *identifier);
};

struct ZRIdentifierS
{
	ZRIdentifierStrategy *strategy;

	size_t nbObj;
};

// ============================================================================

ZRMUSTINLINE
static inline void ZRIDENTIFIER_DONE(ZRIdentifier *identifier)
{
	identifier->strategy->fdone(identifier);

}

ZRMUSTINLINE
static inline void ZRIDENTIFIER_DESTROY(ZRIdentifier *identifier)
{
	identifier->strategy->fdestroy(identifier);
}

ZRMUSTINLINE
static inline size_t ZRIDENTIFIER_NBOBJ(ZRIdentifier *identifier)
{
	return identifier->nbObj;
}

/**
 * Return the Identifier of obj.
 * If it's not present, add a new one in the Identifier memory.
 */
ZRMUSTINLINE
static inline ZRID ZRIDENTIFIER_GETID(ZRIdentifier *identifier, void *obj)
{
	return identifier->strategy->fgetID(identifier, obj);
}

/**
 * Return the canonical representation of obj.
 * If it's not present, add a new one in the Identifier memory.
 */
ZRMUSTINLINE
static inline void* ZRIDENTIFIER_INTERN(ZRIdentifier *identifier, void *obj)
{
	return identifier->strategy->fintern(identifier, obj);
}

ZRMUSTINLINE
static inline void* ZRIDENTIFIER_FROMID(ZRIdentifier *identifier, ZRID id)
{
	return identifier->strategy->ffromID(identifier, id);
}

ZRMUSTINLINE
static inline bool ZRIDENTIFIER_CONTAINS(ZRIdentifier *identifier, void *obj)
{
	return identifier->strategy->fcontains(identifier, obj);
}

ZRMUSTINLINE
static inline bool ZRIDENTIFIER_RELEASE(ZRIdentifier *identifier, void *obj)
{
	return identifier->strategy->frelease(identifier, obj);
}

ZRMUSTINLINE
static inline bool ZRIDENTIFIER_RELEASEID(ZRIdentifier *identifier, ZRID id)
{
	return identifier->strategy->freleaseID(identifier, id);
}

ZRMUSTINLINE
static inline bool ZRIDENTIFIER_RELEASEALL(ZRIdentifier *identifier)
{
	return identifier->strategy->freleaseAll(identifier);
}

// ============================================================================


void ZRIdentifier_done(__ ZRIdentifier *identifier);
void ZRIdentifier_destroy(ZRIdentifier *identifier);

size_t ZRIdentifier_nbObj(ZRIdentifier *identifier);

ZRID __ ZRIdentifier_getID(______ ZRIdentifier *identifier, void *obj);
void* _ ZRIdentifier_intern(_____ ZRIdentifier *identifier, void *obj);
void* _ ZRIdentifier_fromID(_____ ZRIdentifier *identifier, ZRID id);
bool __ ZRIdentifier_contains(___ ZRIdentifier *identifier, void *obj);
bool __ ZRIdentifier_release(____ ZRIdentifier *identifier, void *obj);
bool __ ZRIdentifier_releaseID(__ ZRIdentifier *identifier, ZRID id);
bool __ ZRIdentifier_releaseAll(_ ZRIdentifier *identifier);


#endif
