#include <zrlib/base/MemoryOp.h>

#include <math.h>
#include <string.h>

/**
 * Swap the bytes between 2 memory segment of same size without buffer.
 */
void ZRMemoryOp_swap(void *restrict offseta, void *restrict offsetb, size_t size)
{
	while (size--)
	{
		char const tmp = *(char*)offseta;
		*(char*)offseta = *(char*)offsetb;
		*(char*)offsetb = tmp;
		offseta = (char*)offseta + 1;
		offsetb = (char*)offsetb + 1;
	}
}

/**
 * Swap the bytes between 2 memory segment of same size using an external buffer for the copy.
 */
void ZRMemoryOp_swapB(void *restrict offseta, void *restrict offsetb, size_t size, void *restrict buffer)
{
	memcpy(buffer, offseta, size);
	memcpy(offseta, offsetb, size);
	memcpy(offsetb, buffer, size);
}

/**
 * Shift some bytes inside a memory segment.
 *
 * The order between *offset and *end is not important.
 *
 * @param void *offset First address of the segment
 * @param void *end Last valid address of the segment
 * @param size_t shift Number of bytes to shift
 * @param int right true for shifting to the right, or false to shift to the left
 */
void ZRMemoryOp_shift(void *restrict offset, void *restrict end, size_t shift, bool toTheRight)
{
	if (shift == 0 || offset == end)
		return;

	ptrdiff_t diff = (char*)end - (char*)offset;
	int const nbBytes = diff + 1;
	int const isInverse = diff < 0;

	if (isInverse)
		diff = -diff;

	// The shift go over the memory limit
	if (nbBytes <= shift)
		return;

	// Swap offset <-> end
	if (isInverse)
	{
		void * const tmp = offset;
		offset = end;
		end = tmp;
	}
	char *destination;
	char* source;
	size_t nbBytesCpy;

	if (toTheRight)
	{
		source = offset;
		destination = (char*)offset + shift;
		nbBytesCpy = (char*)end - destination + 1;
	}
	else
	{
		source = (char*)offset + shift;
		destination = offset;
		nbBytesCpy = (char*)end - source + 1;
	}
	size_t const overlapLimit = (nbBytes >> 1) + nbBytes % 2;

	// If no overlap
	if (shift >= overlapLimit)
		memcpy(destination, source, nbBytesCpy);
	else
		memmove(destination, source, nbBytesCpy);
}

/**
 * Rotate some bytes inside a memory segment.
 *
 * The order between *offset and *end is not important.
 *
 * @param void *offset First address of the segment
 * @param void *end Last valid address of the segment
 * @param size_t rotate Number of bytes to rotate
 * @param int right true for rotating to the right, or false to rotate to the left
 */
void ZRMemoryOp_rotate(void *restrict offset, void *restrict end, size_t rotate, bool toTheRight)
{
	if (rotate == 0 || offset == end)
		return;

	ptrdiff_t diff = (char*)end - (char*)offset;
	size_t const nbBytes = diff + 1;

	rotate %= nbBytes;

	if (rotate == 0)
		return;

	int const isInverse = diff < 0;

	if (isInverse)
		diff = -diff;

	if (isInverse)
	{
		void * const tmp = offset;
		offset = end;
		end = tmp;
	}

	// Adapt the rotation side to copy the minimum memory size
	if (rotate > (nbBytes >> 1))
	{
		rotate = nbBytes - rotate;
		toTheRight = !toTheRight;
	}
	char buff[rotate];

	if (toTheRight)
	{
		char * const offsetRotate = (char*)end - rotate + 1;
		memcpy(buff, offsetRotate, rotate);
		memcpy((char*)offset + rotate, offset, nbBytes - rotate);
		memcpy(offset, buff, rotate);
	}
	else
	{
		memcpy(buff, offset, rotate);
		memcpy(offset, (char*)offset + rotate, nbBytes - rotate);
		memcpy((char*)end - rotate + 1, buff, rotate);
	}
}
