cmake_minimum_required(VERSION 3.1)

project(zrlib)

include_directories(${PROJECT_BINARY_DIR})

set(SRCS
	src/base/Allocator/CAllocator.c
	src/base/ArrayOp.c
	src/base/Bits.c
	src/base/MemoryOp.c
	src/base/Vector/Vector.c
	src/base/Vector/Vector2SideStrategy.c
    )
    
set(HEADERS
	zrlib/base/Allocator/Allocator.h
	zrlib/base/Allocator/CAllocator.h
	zrlib/base/ArrayOp.h
	zrlib/base/Bits.h
	zrlib/base/macro.h
	zrlib/base/MemoryOp.h
	zrlib/base/Vector/Vector.h
	zrlib/base/Vector/Vector2SideStrategy.h
	zrlib/config.h
    )

add_library(zrlib ${SRCS} ${HEADERS})
set_property(TARGET zrlib PROPERTY C_STANDARD 11)
