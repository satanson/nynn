#ifndef NYNN_MM_META_BY_SATANSON
#define NYNN_MM_META_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_chunk.h>
#include<nynn_mm_cache.h>
using namespace nynn::mm::common;
using namespace nynn::mm;

namespace nynn{namespace mm{
//config
//

enum{
	PATH_SIZE=64,
	CHUNK_DIR_SIZE=64,
	CHUNK_ENTRY_MAX=128,
	CHUNK_LOCKID_MAX=32,
	LOCKSLOT_MAX=128,

	CHUNK_ENTRY_FLAG_LOCAL=1,

	DUMMY
};

struct chunk_entry_t;
class  meta_t;

struct chunk_entry_t{
	char _path[PATH_SIZE];
	uint8_t  _flag;	
	uint64_t _minvertex;
	uint64_t _maxvertex;
	uint32_t _where;
};

typedef chunk_t<> chunk_type;
typedef cache_t<> cache_type;

class meta_t:public shm_allocator_t{
public:
	static meta_t* get_meta(void* shm,size_t size,const string &chunkdir)
	{
		return new(shm)meta_t(chunkdir);
	}
	~meta_t()
	{	



		//destroy locks
		semid1_t tmp0(_metalockid);
		semid1_t tmp1(_cachelockid);
		for (size_t i=0;i<CHUNK_LOCKID_MAX;i++){
			semid1_t tmp2(_chunklockid[i]);
		}
	}

	int  get_cacheshmid()const{return _cacheshmid;}
	size_t get_chunkentrynum()const{return _nchunkentry;}
	chunk_entry_t* get_chunkentry(int i){return &_chunkentry[i];}
private:
	meta_t(const string& chunkdir)
	{

		//assert(chunkdir.size()<sizeof(_chunkdir));
		if (!(chunkdir.size()<sizeof(_chunkdir))){
			log_a("assert(strlen(chunkdir)<sizeof(_chunkdir):failure");
			THROW_ERROR_WITH_ERRNO(EINVAL);
		}
		memset(_chunkdir,0,sizeof(_chunkdir));
		strcpy(_chunkdir,chunkdir.c_str());

		//initialize cacheshmid
		shmid_t cacheshmid(sizeof(cache_type));
		_cacheshmid=cacheshmid.get_shmid();

		//initialize chunk entries.
		//get local chunk entries.
		memset(_chunkentry,0,sizeof(_chunkentry));

		glob_t g;
		g.gl_offs=0;
		string pat=chunkdir+"/chunk*.dat";
		glob(pat.c_str(),0,NULL,&g);
		//assert(g.gl_pathc<=CHUNK_ENTRY_MAX);
		if (!(g.gl_pathc<=CHUNK_ENTRY_MAX)){
			log_a("assert(g.gl_pathc<=CHUNK_ENTRY_MAX):failue");
			THROW_ERROR_WITH_ERRNO(EINVAL);
		}

		_nchunkentry=g.gl_pathc;
		for (size_t i=0;i<_nchunkentry;i++){
			//assert(strlen(g.gl_pathv[i])<PATH_SIZE);
			if (!(strlen(g.gl_pathv[i])<PATH_SIZE)){
				log_a("assert(strlen(g.gl_pathv[i])<PATH_SIZE):failue");
				THROW_ERROR_WITH_ERRNO(EINVAL);
			}
			strcpy(_chunkentry[i]._path,g.gl_pathv[i]);
			_chunkentry[i]._flag|=CHUNK_ENTRY_FLAG_LOCAL;
			_chunkentry[i]._where=i;
			unique_ptr<mmap_file_t> chunkmf(new mmap_file_t(_chunkentry[i]._path));
			chunk_type* chk=static_cast<chunk_type*>(chunkmf->get_base());
			_chunkentry[i]._minvertex=chk->get_minvertex();
			_chunkentry[i]._maxvertex=chk->get_maxvertex();
		}

		globfree(&g);

		
		//initialize locks.	
		_metalockid=semid0_t(1).get_semid();
		_cachelockid=semid0_t(LOCKSLOT_MAX).get_semid();
		
		for (size_t i=0;i<CHUNK_LOCKID_MAX;i++)
			_chunklockid[i]=semid0_t(LOCKSLOT_MAX).get_semid();
	}

	meta_t(const meta_t&);
	meta_t& operator=(const meta_t&);

	char _chunkdir[CHUNK_DIR_SIZE];
	int _cacheshmid;
	size_t _nchunkentry;
	chunk_entry_t _chunkentry[CHUNK_ENTRY_MAX];
	
	int _metalockid;//1*1 locks for meta.
	int _cachelockid;//1*LOCKSLOT_MAX locks for cache.
	int _chunklockid[CHUNK_LOCKID_MAX];//32*LOCKSLOT_MAX locks for chunk
};
}}
#endif
