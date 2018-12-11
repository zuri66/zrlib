/**
 * @author zuri
 * @date dimanche 25 novembre 2018, 23:59:47 (UTC+0100)
 */

#ifndef ZRMEMORY_OP_H
#define ZRMEMORY_OP_H

#include <stddef.h>
#include <stdbool.h>

void ZRMemoryOp_swap   (void *restrict offseta, void *restrict offsetb, size_t size);
void ZRMemoryOp_swapB  (void *restrict offseta, void *restrict offsetb, size_t size, void *restrict buffer);

void ZRMemoryOp_shift (void *restrict offset, void *restrict end, size_t shift , bool toTheRight);
void ZRMemoryOp_rotate(void *restrict offset, void *restrict end, size_t rotate, bool toTheRight);

#endif
