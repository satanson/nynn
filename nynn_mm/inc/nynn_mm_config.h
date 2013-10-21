#ifndef NYNN_MM_CONFIG_BY_SATANSON
#define NYNN_MM_CONFIG_BY_SATANSON
#include<nynn_mm_common.h>

using namespace nynn::mm::common;
namespace nynn{namespace mm{
	typedef uint64_t VertexNoType;
	typedef uint64_t EdgeNoType; 
	typedef uint32_t ChunkBlockNoType;
	typedef uint16_t CacheBlockNoType;
	typedef uint8_t  CacheListLenType;
	typedef uint64_t EdgeNumType;
	enum{
		chunk_block_size = 1<<9,
		chunk_block_max  = 1<<23,
		chunk_vertex_max = 1<<23,

		meta_chunk_path_size = 64,
		meta_chunk_entry_max = 128,
		meta_chunk_lock_max  = 32,
		meta_chunk_lock_slot_max = 128,
		meta_cache_lock_slot_max = 128,

		cache_block_size = 1<<9,
		cache_head_max   = 1<<10,
		cache_block_max  = 2**16
	}
};
}}
#endif
