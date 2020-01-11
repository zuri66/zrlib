/**
 * @author zuri
 * @date mardi 18 décembre 2018, 22:46:00 (UTC+0100)
 */

#ifndef ZRMACRO_H
#define ZRMACRO_H

#define ZRSIZE_UNKNOW (~(size_t)0)
#define ZRTOSTRING(V) #V

#define ZRCONCAT(A,B) ZRCONCAT_(A,B)
#define ZRCONCAT_(A,B) A ## B

#define ZRMIN(a,b) ((a) < (b) ? (a) : (b))

#define ZRCARRAY_NBOBJ(array) (sizeof(array)/sizeof(*array))

//#define ZRSTRUCT_FAM

/**
 * Size for structure with a flexible array member which conserve the structure alignment.
 * The size of the FAM must be counted in the structSize argument.
 */
static inline size_t ZRSTRUCTSIZE_FAM_PAD(size_t structSize, size_t alignment)
{
	size_t const rest = structSize % alignment;

	if(rest)
		return structSize + alignment - rest;

	return structSize;
}

#endif
