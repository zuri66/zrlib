/**
 * @author zuri
 * @date mardi 18 décembre 2018, 22:46:00 (UTC+0100)
 */

#ifndef ZRMACRO_H
#define ZRMACRO_H

#include <zrlib/syntax_pad.h>
#include <zrlib/base/macro/macro_rec.h>

#include <stdalign.h>
#include <stddef.h>

typedef size_t ZRID;
#define ZRID_ABSENT SIZE_MAX

typedef struct
{
	alignas(max_align_t) char buffer[2048];
} ZRInitInfos_t[1];

typedef size_t (*zrfhash)(void *a);
typedef size_t (*zrfuhash)(void *a, void *data);

typedef int (*zrfcmp)(void *a, void *b);
typedef int (*zrfucmp)(void *a, void *b, void *data);

#define ZRSWAP(type,a,b) ZRBLOCK( \
	type _c; \
	_c = a, a = b, b = _c; \
)

#define ZRPTYPE_CPY(a,b) memcpy((a), (b), sizeof(*a))
#define ZRPTYPE_0(a) memset((a), 0, sizeof(*a))
#define ZRPTYPE_OBJECTP(a) ZROBJECTP_DEF( ZROBJINFOS_DEF(alignof(*a), sizeof(*a)), a)

#define ZRSTRING_OBJECTP(S) ZROBJECTP_DEF( ZROBJINFOS_DEF(alignof(char), strlen(S) + 1), S )

#define ZRTYPE_SIZE_ALIGNMENT(T) sizeof(T), __alignof(T)
#define ZRTYPE_ALIGNMENT_SIZE(T) __alignof(T), sizeof(T)
#define ZRTYPE_OBJINFOS(T) ZROBJINFOS_DEF(__alignof(T), sizeof(T))
#define ZRTYPENB_OBJINFOS(T,NB) ZROBJINFOS_DEF(__alignof(T), sizeof(T) * (NB))
#define ZRTYPE_OBJALIGNINFOS(T) ZROBJALIGNINFOS_DEF(0, __alignof(T), sizeof(T))
#define ZRTYPENB_OBJALIGNINFOS(T,NB) ZROBJALIGNINFOS_DEF(0, __alignof(T), sizeof(T) * (NB))

#define ZRDEREF(P) *(P)
#define ZRADDR(P)  &(P)

#define ZRSIZE_UNKNOWN SIZE_MAX
#define ZRTOSTRING(V) #V

#define ZRCONCAT(_1,_2)         ZRCONCAT_2(_1,_2)
#define ZRCONCAT_2(_1,_2)       ZRCONCAT_(_1,_2)
#define ZRCONCAT_3(_1,_2,_3)    ZRCONCAT_2(_1, ZRCONCAT_2(_2,_3))
#define ZRCONCAT_4(_1,_2,_3,_4) ZRCONCAT_2(_1, ZRCONCAT_3(_2,_3,_4))
#define ZRCONCAT_(A,B) A ## B

#define ZRMAX(...) ZRCONCAT(ZRMAX_,ZRMACRO_NARGS(__VA_ARGS__))(__VA_ARGS__)
#define ZRMIN(...) ZRCONCAT(ZRMIN_,ZRMACRO_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define ZRMIN_2(a,b) ((a) < (b) ? (a) : (b))
#define ZRMIN_3(a,b,c) ZRMIN_2(a,ZRMIN_2(b,c))
#define ZRMIN_4(a,b,c,d) ZRMIN_2(ZRMIN_2(a,b),ZRMIN_2(c,d))

#define ZRMAX_2(a,b) ((a) > (b) ? (a) : (b))
#define ZRMAX_3(a,b,c) ZRMAX_2(a,ZRMAX_2(b,c))
#define ZRMAX_4(a,b,c,d) ZRMAX_2(ZRMAX_2(a,b),ZRMAX_2(c,d))

#define ZRISPOW2(I) (((I) & ((I) - 1)) == 0)
#define ZRISPOW2SAFE(I) ((I) > 0 && ZRISPOW2(I))

#define ZRCARRAY_NBOBJ(array) (sizeof(array)/sizeof(*array))
#define ZRCARRAY_CPY(A,B) memcpy(A, B, sizeof(A))

#ifdef __GNUC__
#define ZRMUSTINLINE __attribute__((always_inline))
#else
#define ZRMUSTINLINE
#endif

#define ZRCODE(...) __VA_ARGS__

#define ZRBLOCK(...) \
do { \
	__VA_ARGS__ \
}while(0)


#define ZRCARRAY_XCPY(dest,src,nb,X) ZRBLOCK( \
	for(size_t _i = 0, _c = (nb) ; _i < _c ; _i++) \
		dest[_i] = X(src[_i]); \
	)

#define ZRCARRAY_XXCPY(dest,src,nb,SET) ZRBLOCK( \
	for(size_t _i = 0, _c = (nb) ; _i < _c ; _i++) \
		SET(dest[_i], src[_i]); \
	)

#define ZRCARRAY_CHECKFORCPY(offset, nbObj, maxNbCpy, CODE_ERROR, CODE_END) ZRBLOCK( \
	size_t const _offset = (offset); \
	size_t const _nbObj = (nbObj); \
	\
	if (_offset >= _nbObj) \
		CODE_ERROR; \
	\
	size_t const _nbFromOffset = _nbObj - _offset; \
	size_t const _maxNbCpy = (maxNbCpy); \
	size_t const _nb = ZRMIN(_maxNbCpy, _nbFromOffset); \
	\
	CODE_END; \
)

#define ZRCARRAY_SAFECPY(offset, nbObj, maxNbCpy, i, CODE_ERROR, CODE, CODE_END) \
	ZRCARRAY_CHECKFORCPY(offset, nbObj, maxNbCpy, CODE_ERROR, ZRCODE( \
		for (size_t i = 0; i < _nb; i++) { \
			CODE; \
		} \
		CODE_END \
	) \
)

//(T, T** pointers, TS, TS* source, size_t nbObj)
#define ZRCARRAY_TOPOINTERS(TP, pointers, TS, source, nbObj) ZRBLOCK( \
	size_t _nb = (nbObj); \
	TP** P = (pointers); \
	TS* S = (source); \
	\
	while (_nb--) \
		P[_nb] = (TP*)(&S[_nb]); \
)

//(TO, TO* offset, TS, TS** source, size_t nbObj)
#define ZRCARRAY_FROMPOINTERS(TO, offset, TS, source, nbObj) ZRBLOCK( \
	size_t _nb = (nbObj); \
	TO* O = (offset); \
	TS** S = (source); \
	\
	while (_nb--) \
		O[_nb] = *S[_nb]; \
)

//(TO, TO** offset, TS, TS** source, size_t nbObj)
#define ZRCARRAY_CPYPOINTERS(TO, offset, TS, source, nbObj) ZRBLOCK( \
	size_t _nb = (nbObj); \
	TO** O = (offset); \
	TS** S = (source); \
	\
	while (_nb--) \
		O[_nb] = (TO*)S[_nb]; \
)

//(T, T** pointers, T* source, size_t nbObj)
#define ZRCARRAY_TOPOINTERSDATA(T, pointers, source, nbObj) ZRBLOCK( \
	size_t _nb = (nbObj); \
	T** P = (pointers); \
	T* S = (source); \
	\
	while (_nb--) \
		*P[_nb] = S[_nb]; \
)

//(T, T* offset, T** source, size_t nbObj)
#define ZRCARRAY_FROMPOINTERSDATA(T, offset, source, nbObj) ZRBLOCK( \
	size_t _nb = (nbObj); \
	T* O = (offset); \
	T** S = (source); \
	\
	while (_nb--) \
		O[_nb] = *S[_nb]; \
)
#endif
