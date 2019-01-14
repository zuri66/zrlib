/**
 * @author zuri
 * @date mardi 18 décembre 2018, 22:46:00 (UTC+0100)
 */

#define ZRLIB_SIZE_UNKNOW (~(size_t)0)
#define TOSTRING(V) #V

#define ZRLIB_MIN(a,b) ((a) < (b) ? (a) : (b))

#define ZRLIB_ARRAY_NBOBJ(array) (sizeof(array)/sizeof(*array))
