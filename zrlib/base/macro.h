/**
 * @author zuri
 * @date mardi 18 décembre 2018, 22:46:00 (UTC+0100)
 */

#ifndef ZRMACRO_H
#define ZRMACRO_H

#include <stddef.h>

#define ZRSIZE_UNKNOW (~(size_t)0)
#define ZRTOSTRING(V) #V

#define ZRCONCAT(A,B) ZRCONCAT_(A,B)
#define ZRCONCAT_(A,B) A ## B

#define ZRMACRO_NARGS(...) ZRMACRO_NARGS_(__VA_ARGS__, ZRMACRO_REVERSE_NARGS())
#define ZRMACRO_NARGS_(...) ZRMACRO_NARGS__(__VA_ARGS__)
#define ZRMACRO_NARGS__(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,N,_...) N
#define ZRMACRO_REVERSE_NARGS() 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0


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


#ifdef __GNUC__
#define ZRMUSTINLINE __attribute__((always_inline))
#else
#define ZRMUSTINLINE
#endif

//(T, T** pointers, TS, TS* source, size_t nbObj)
#define ZRCARRAY_TOPOINTERS(TP, pointers, TS, source, nbObj) \
do \
{ \
	size_t _nb = (nbObj); \
	TP** P = (pointers); \
	TS* S = (source); \
	\
	while (_nb--) \
		P[_nb] = (TP*)(&S[_nb]); \
} \
while(0)

//(T, T** pointers, T* source, size_t nbObj)
#define ZRCARRAY_TOPOINTERSDATA(T, pointers, source, nbObj) \
do \
{ \
	size_t _nb = (nbObj); \
	T** P = (pointers); \
	T* S = (source); \
	\
	while (_nb--) \
		*P[_nb] = S[_nb]; \
} \
while(0)

//(T, T* offset, size_t nbObj, T** source)
#define ZRCARRAY_FROMPOINTERSDATA(T, offset, source, nbObj) \
do \
{ \
	size_t _nb = (nbObj); \
	T* O = (offset); \
	T** S = (source); \
	\
	while (_nb--) \
		O[_nb] = *S[_nb]; \
} \
while(0)

#endif
