#ifndef NYNN_MM_CONFIG_BY_SATANSON
#define NYNN_MM_CONFIG_BY_SATANSON

namespace nynn{namespace mm{
	enum{

		block_size = 1<<9,

		chunk_block_max  = 1<<11,
		chunk_vertex_max = 1<<11,

		meta_chunk_path_size = 64,
		meta_chunk_entry_max = 128,
		meta_chunk_lock_max  = 32,
		meta_chunk_lock_slot_max = 128,

		cache_head_max   = 1<<10,
		cache_block_max  = 1<<16
	};
}}
#endif