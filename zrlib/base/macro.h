/**
 * @author zuri
 * @date mardi 18 décembre 2018, 22:46:00 (UTC+0100)
 */

#ifndef ZRMACRO_H
#define ZRMACRO_H

#include <zrlib/syntax_pad.h>

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

#define ZRNARGS ZRMACRO_NARGS

#define ZRMACRO_NARGS(...) ZRMACRO_NARGS_(__VA_ARGS__, ZRMACRO_REVERSE_NARGS())
#define ZRMACRO_NARGS_(...) ZRMACRO_NARGS__(__VA_ARGS__)
#define ZRMACRO_NARGS__(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,N,_...) N
#define ZRMACRO_REVERSE_NARGS() 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

#define ZRNARGS_MAX 15


/* https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/ */
#define ZRARGS_HAS_COMMA(...) ZRMACRO_NARGS__(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define _TRIGGER_PARENTHESIS_(...) ,
#define _IS_EMPTY_CASE_0001 ,
#define ZRARGS_EMPTY(...) ZRARGS_HAS_COMMA(ZRCONCAT_2(_IS_EMPTY_CASE_, _ZRARGS_EMPTY_BITS(__VA_ARGS__)))
#define ZRARGS_NEMPTY(...) ZRCOMPL(ZRARGS_EMPTY(__VA_ARGS__))

/* test if there is just one argument, eventually an empty one */
/* test if _TRIGGER_PARENTHESIS_ together with the argument
 /* test if the argument together with a adds a comma */
/* test if placing it between _TRIGGER_PARENTHESIS_ and the parenthesis adds a comma */
#define _ZRARGS_EMPTY_BITS(...) ZRCONCAT_4(\
	ZRARGS_HAS_COMMA(__VA_ARGS__), \
	ZRARGS_HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__), \
	ZRARGS_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
	ZRARGS_HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)))

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

/* https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms */
#define ZREMPTY()
#define ZRFORGET(...)
#define ZRDEFER(name) name ZREMPTY()
#define ZREXPAND(...) __VA_ARGS__
#define ZROBSTRUCT(NAME) NAME ZRDEFER(ZREMPTY)()

#define ZREVAL(...)  ZREVAL1(ZREVAL1(__VA_ARGS__))
#define ZREVAL1(...) ZREVAL2(ZREVAL2(__VA_ARGS__))
#define ZREVAL2(...) ZREVAL3(ZREVAL3(__VA_ARGS__))
#define ZREVAL3(...) ZREVAL4(ZREVAL4(__VA_ARGS__))
#define ZREVAL4(...) ZREVAL5(ZREVAL5(__VA_ARGS__))
#define ZREVAL5(...) ZREVAL_(ZREVAL_(__VA_ARGS__))
#define ZREVAL_(...) __VA_ARGS__

#define ZRCHECK_N(x, n, ...) n
#define ZRCHECK(...) ZRCHECK_N(__VA_ARGS__, 0)
#define ZRPROBE(x) x, 1

#define ZRNOT(x) ZRCHECK(ZRCONCAT_2(ZRNOT_, x)())
#define ZRNOT_0() ZRPROBE(~)
#define ZRNOT_() ZRPROBE(~)

#define ZRCOMPL(b) ZRCONCAT_2(ZRCOMPL_, b)
#define ZRCOMPL_0 1
#define ZRCOMPL_1 0

#define ZRGT_1(N) ZRCOMPL(ZRCHECK(ZRCONCAT(ZRGT_1_, N)()))
#define ZRGT_1_() ZRPROBE(~)
#define ZRGT_1_0() ZRPROBE(~)
#define ZRGT_1_1() ZRPROBE(~)

#define ZRBOOL(x) ZRCOMPL(ZRNOT(x))

#define ZRBITOR(x) ZRCONCAT_2(ZRBITOR, x)
#define ZRBITOR_0(y) y
#define ZRBITOR_1(y) 1

#define ZRBITAND(x) ZRCONCAT_2(ZRBITAND_, x)
#define ZRBITAND_0(y) 0
#define ZRBITAND_1(y) y

#define ZRAND(...) ZRCONCAT(ZRAND_,ZRNARGS(__VA_ARGS__))(__VA_ARGS__)
#define ZRAND_2(_1,_2) ZRCONCAT(ZRBITAND_,ZRBOOL(_1))(ZRBOOL(_2))

#define ZROR(...) ZRCONCAT(ZROR_,ZRNARGS(__VA_ARGS__))(__VA_ARGS__)
#define ZROR_2(_1,_2) ZRCONCAT(ZRBITOR_,ZRBOOL(_1))(ZRBOOL(_2))

#define ZRINC(x) ZRCONCAT_2(ZRINC_, x)
#define ZRINC_0 1
#define ZRINC_1 2
#define ZRINC_2 3
#define ZRINC_3 4
#define ZRINC_4 5
#define ZRINC_5 6
#define ZRINC_6 7
#define ZRINC_7 8
#define ZRINC_8 9
#define ZRINC_9 10

#define ZRDEC(x) ZRCONCAT_2(ZRDEC_, x)
#define ZRDEC_0 0
#define ZRDEC_1 0
#define ZRDEC_2 1
#define ZRDEC_3 2
#define ZRDEC_4 3
#define ZRDEC_5 4
#define ZRDEC_6 5
#define ZRDEC_7 6
#define ZRDEC_8 7
#define ZRDEC_9 8
#define ZRDEC_10 9


#define ZRIIF(c) ZRCONCAT_2(ZRIIF_, c)
#define ZRIIF_0(t, ...) __VA_ARGS__
#define ZRIIF_1(t, ...) t

#define ZRIF(c) ZRIIF(ZRBOOL(c))
#define ZRWHEN(c) ZRIF(c)(ZREXPAND, ZRFORGET)

#define ZRREPEAT(count, macro, ...) \
    ZRWHEN(count) \
    ( \
        ZROBSTRUCT(ZRREPEAT_INDIRECT) () \
        ( \
            ZRDEC(count), macro, __VA_ARGS__ \
        ) \
        ZROBSTRUCT(macro) \
        ( \
            ZRDEC(count), __VA_ARGS__ \
        ) \
    )
#define ZRREPEAT_INDIRECT() ZRREPEAT
#define ZRREPEAT_E(...) ZREVAL(ZRREPEAT(__VA_ARGS__))

#define ZRWHILE(pred, op, ...) \
    ZRIF(pred(__VA_ARGS__)) \
    ( \
        ZROBSTRUCT(ZRWHILE_INDIRECT) () \
        ( \
            pred, op, op(__VA_ARGS__) \
        ), \
        __VA_ARGS__ \
    )
#define ZRWHILE_INDIRECT() ZRWHILE
#define ZRWHILE_E(...) ZREVAL(ZRWHILE(__VA_ARGS__))

#define ZRARGS_XEVEN(X,V, ...) \
	X(V) \
	ZRWHEN(ZRGT_1(ZRNARGS(__VA_ARGS__))) \
	(, \
		ZROBSTRUCT(ZRARGS_XODD_INDIRECT) () \
		(X,__VA_ARGS__) \
	)
#define ZRARGS_XODD(X,V, ...) ZRARGS_XEVEN(X,__VA_ARGS__)
#define ZRARGS_XODD_INDIRECT()  ZRARGS_XODD

#define ZRARGS_EVENODD_EXPAND(...) __VA_ARGS__
#define ZRARGS_EVEN(...) ZRARGS_XEVEN(ZRARGS_EVENODD_EXPAND,__VA_ARGS__)
#define ZRARGS_ODD(...)  ZRARGS_XODD(ZRARGS_EVENODD_EXPAND,__VA_ARGS__)

#define ZRARGS_XEVEN_E(X,...) ZREVAL(ZRARGS_XEVEN(X,__VA_ARGS__))
#define ZRARGS_XODD_E(X,...)  ZREVAL(ZRARGS_XODD(X,__VA_ARGS__))
#define ZRARGS_EVEN_E(...)    ZREVAL(ZRARGS_EVEN(__VA_ARGS__))
#define ZRARGS_ODD_E(...)     ZREVAL(ZRARGS_ODD(__VA_ARGS__))

#define _ZRARGS_XCAPPLY(X, count, V, ...) \
	ZRWHEN(ZRARGS_NEMPTY(V))( \
		X(count,V) \
		ZROBSTRUCT(_ZRARGS_XCAPPLY_INDIRECT) () \
		( \
			X, ZRINC(count), __VA_ARGS__ \
		) \
	)
#define _ZRARGS_XCAPPLY_INDIRECT() _ZRARGS_XCAPPLY
#define ZRARGS_XCAPPLY(X,...) _ZRARGS_XCAPPLY(X,0,__VA_ARGS__)
#define ZRARGS_XCAPPLY_E(X,...) ZREVAL(ZRARGS_XCAPPLY(X,__VA_ARGS__))


#define ZRCARRAY_XCPY(dest,src,nb,X) ZRBLOCK( \
	for(size_t _i = 0, _c = (nb) ; _i < _c ; _i++) \
		dest[_i] = X(src[_i]); \
	)

#define ZRCARRAY_XXCPY(dest,src,nb,X,SET) ZRBLOCK( \
	for(size_t _i = 0, _c = (nb) ; _i < _c ; _i++) \
		SET(dest[_i], X(src[_i])); \
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
