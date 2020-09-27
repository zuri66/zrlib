cmake_minimum_required(VERSION 3.1)

project(zrlib)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${PROJECT_BINARY_DIR})

add_compile_options(-mbmi -mlzcnt -Winline)

set(SRCS
	src/base/Algorithm/fcmp.c
	src/base/Algorithm/sort.c
	src/base/AddressOp.c
	src/base/Allocator/CAllocator.c
	src/base/ArrayOp.c
	src/base/ReserveOp_bits.c
	src/base/ReserveOp_chunk.c
	src/base/ReserveOp_list.c
	src/base/Bits/Bits.c
	src/base/Iterator/FilterIterator.c
	src/base/Iterator/Iterator.c
	src/base/List/List.c
	src/base/MemoryOp.c
	src/base/ResizeOp.c
	src/base/struct.c
	src/base/Graph/SimpleGraph.c
	src/base/Graph/SimpleGraphBuilder.c
	src/base/Graph/Graph.c
	src/base/Graph/Tree/SimpleTree.c
	src/base/Graph/Tree/SimpleTreeBuilder.c
	src/base/Graph/Tree/Tree.c
	src/base/Identifier/Identifier.c
	src/base/Vector/Vector.c
	src/base/Vector/Vector2SideStrategy.c
	src/base/Map/HashTable.c
	src/base/Map/Map.c
	src/base/Map/VectorMap.c
	src/base/MemoryPool/MemoryPool.c
	src/base/MemoryPool/MPoolDynamicStrategy.c
	src/base/MemoryPool/MPoolReserve.c
    )
    
set(HEADERS
	zrlib/base/Algorithm/fcmp.h
	zrlib/base/Algorithm/sort.h
	zrlib/base/AddressOp.h
	zrlib/base/Allocator/Allocator.h
	zrlib/base/Allocator/CAllocator.h
	zrlib/base/Array.h
	zrlib/base/ArrayOp.h
	zrlib/base/ReserveOp_bits.h
	zrlib/base/ReserveOp_chunk.h
	zrlib/base/ReserveOp_list.h
	zrlib/base/Bits/Bits.h
	zrlib/base/Iterator/FilterIterator.h
	zrlib/base/Iterator/Iterator.h
	zrlib/base/macro.h
	zrlib/base/struct.h
	zrlib/base/List/List.h
	zrlib/base/MemoryOp.h
	zrlib/base/ResizeOp.h
	zrlib/base/Graph/SimpleGraph.h
	zrlib/base/Graph/Graph.h
	zrlib/base/Graph/Tree/SimpleTree.h
	zrlib/base/Graph/Tree/Tree.h
	zrlib/base/Identifier/Identifier.h
	zrlib/base/Vector/Vector.h
	zrlib/base/Vector/Vector2SideStrategy.h
	zrlib/base/Map/HashTable.h
	zrlib/base/Map/Map.h
	zrlib/base/Map/VectorMap.h
	zrlib/base/MemoryPool/MemoryPool.h
	zrlib/base/MemoryPool/MPoolDynamicStrategy.h
	zrlib/base/MemoryPool/MPoolReserve.h
	zrlib/config.h
    )

add_compile_options(-std=c11)
add_library(zrlib ${SRCS} ${HEADERS})
set_property(TARGET zrlib PROPERTY C_STANDARD 11)
