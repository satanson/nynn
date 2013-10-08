#ifndef NYNN_MM_META_BY_SATANSON
#define NYNN_MM_META_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_local.h>
#include<nynn_mm_cache.h>
using namespace nynn::mm::common;
using namespace nynn::mm::local;
using namespace nynn::mm::cache;

namespace nynn{namespace mm{namespace meta{
//config
enum{
	BLOCKSZ=1<<9,
	NBLOCK_IN_SUBCHUNK=1<<23,
	NVERTEX_IN_SUBCHUNK=1<<23,
	NENTRY_IN_CHUNK=1<<8,
	NBLOCK_IN_CACHE=1<<16,
	NHEAD_IN_CACHE=1<<10,
	MAX_NSUBCHUNK=1<<5,

	DUMMY
};

typedef chunk_t<NENTRY_IN_CHUNK> chunk_type;
typedef subchunk_t<BLOCKSZ,NBLOCK_IN_SUBCHUNK,NVERTEX_IN_SUBCHUNK> subchunk_type;
typedef cache_t<BLOCKSZ,NBLOCK_IN_CACHE,NHEAD_IN_CACHE> cache_type;

class meta_t{
public:
private:
	meta_t(const string &path)_dir(path)
	{
		string chunk_dat_path=_dir+"chunk.dat";
		string subchunk_dat_pattern=_dir+"subchunks/subchunk*.dat";
		string cache_dat_path=_dir+"cache.dat";
	}

	meta_t(const meta_t&);
	meta_t& operator(const meta_t&);

	// /path/to/dir/
	string _dir;
	// /path/to/dir/chunk.dat
	chunk_type *_chunk;
	// /path/to/dir/subchunk*.dat
	subchunk_type *_subchunks[MAX_NSUBCHUNK];
	// /path/to/dir/cache.dat
	cache_type *_cache;
	size_t _nsubchunk;
};

}}}
#endif
