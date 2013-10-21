#ifndef NYNN_MM_CONTEXT_BY_SATANSON
#define NYNN_MM_CONTEXT_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_chunk.h>
#include<nynn_mm_cache.h>
#include<nynn_mm_meta.h>
using namespace nynn::mm::common;
using namespace nynn::mm;

namespace nynn{namespace mm{

enum{
	CHUNK_MAX=128
};
typedef meta_t meta_type;
typedef chunk_t<> chunk_type;
typedef cache_t<> cache_type;

struct once_t{
	int _refc;
	int _shmid;
	int _length;
};

void exit_on_recv_signal(int signo)
{
	exit(signo);
}

void add_signal_handler(int signo,void (*handler)(int)){
	struct sigaction sigact;
	sigact.sa_handler=handler;
	sigaction(signo,&sigact,0);
}

class context_t{
public:
	context_t(const string& dir): _dir(dir),_lockname("nynn_mm.lock"),
		_chunkname("chunk/"),_oncename("nynn_mm.once")
	{
		add_signal_handler(SIGINT,exit_on_recv_signal);
		add_signal_handler(SIGABRT,exit_on_recv_signal);

		string lockpath=_dir+"/"+_lockname;
		string oncepath=_dir+"/"+_oncename;
		string chunkdir=_dir+"/"+_oncename;
		
		//require file lock.
		log_d("lock file:%s",lockpath.c_str());
		unique_ptr<flock_t> flockptr(new fwlock_t(lockpath));
		raii_flock_t require(flockptr.get());

		//test whether once file exists.
		//if exists,initialize multiprocess-shared non-persistent resources;
		//otherwise,just obtain the handles to resources.
		if (file_exist(oncepath)){
			log_d("obtain resources.");
			mmap_file_t once_mf(oncepath);
			once_t *once=static_cast<once_t*>(once_mf.get_base());
			once->_refc++;

			//obtain initialized meta.
			_metashm.reset(new shm_t(once->_shmid,sizeof(meta_type)));
			_meta=static_cast<meta_type*>(_metashm->get_base());

			//obtain initialized cache.
			_cacheshm.reset(new shm_t(_meta->get_cacheshmid()));
			_cache=static_cast<cache_type*>(_cacheshm->get_base());

		}else{
			log_d("initializing resources.");
			mmap_file_t once_mf(oncepath,sizeof(once_t));
			once_t *once=static_cast<once_t*>(once_mf.get_base());
			once->_refc=1;

			shmid_t metashmid(sizeof(meta_type));
			_metashm.reset(new shm_t(metashmid));
			once->_shmid=_metashm->get_shmid();	
			once->_length=_metashm->get_length();

			//initialize meta.
			_meta=meta_type::get_meta(_metashm->get_base(),sizeof(meta_type),chunkdir);

			//initialize cache.
			_cacheshm.reset(new shm_t(_meta->get_cacheshmid(),sizeof(cache_type)));
			_cache=cache_type::get_cache(_cacheshm->get_base());

		}
		//initialize chunks.
		size_t nchunkentry=_meta->get_chunkentrynum();
		for(size_t i=0;i<nchunkentry;i++){
			chunk_entry_t *chk_entry=_meta->get_chunkentry(i);
			if (chk_entry->_flag & CHUNK_ENTRY_FLAG_LOCAL){
				_chunkmf[chk_entry->_where].reset(new mmap_file_t(chk_entry->_path));
				void* base=_chunkmf[chk_entry->_where]->get_base();
				_chunk[chk_entry->_where]=static_cast<chunk_type*>(base);
			}
		}
	}
	~context_t()
	{
		string lockpath=_dir+"/"+_lockname;
		string oncepath=_dir+"/"+_oncename;
		
		//require file lock.
		unique_ptr<flock_t> flockptr(new fwlock_t(lockpath));
		raii_flock_t require(flockptr.get());
		mmap_file_t once_mf(oncepath);
		once_t *once=static_cast<once_t*>(once_mf.get_base());
		once->_refc--;
		
		if (once->_refc==0) {
			//release meta.
			unique_ptr<meta_type> tmpmeta(_meta);

			//release cache.
			unique_ptr<cache_type> tmpcache(_cache);
			
			//release chunks
			unique_ptr<chunk_type> tmpchunk[CHUNK_MAX];
			size_t nchunkentry=_meta->get_chunkentrynum();
			for (size_t i=0;i<nchunkentry;i++){
				chunk_entry_t *chk_entry=_meta->get_chunkentry(i);
				if (chk_entry->_flag & CHUNK_ENTRY_FLAG_LOCAL){
					tmpchunk[chk_entry->_where].reset(_chunk[chk_entry->_where]);
				}
			}
			if(remove(oncepath.c_str())!=0)THROW_ERROR;
		}
		log_d("");
	}

	meta_type* get_meta()const{return _meta;}
	static context_t * get_context()
	{
		if (_context.get()==NULL){
			char * data_dir=getenv("NYNN_MM_DATA_DIR");
			if(data_dir==NULL){
				log_w("environment NYNN_MM_DATA_DIR is not set!");
				exit(0);
			}
			_context.reset(new context_t(data_dir));
		}
		log_d("");
		return _context.get();
	}
private:

	context_t(const context_t&);
	context_t& operator=(const context_t&);

	string _dir;
	string _lockname;
	string _oncename;
	string _chunkname;

	// "_dir/nynn_mm.lock" used as flock for controling all shared non-persistent data.
	// "_dir/chunk" contain all chunks(local graph data).
	// "_dir/nynn_mm.once" used as tag for all shared data initialized only once.
	
	meta_type  *_meta;
	cache_type *_cache;
	chunk_type *_chunk[CHUNK_MAX];

	unique_ptr<shm_t> _metashm;
	unique_ptr<shm_t> _cacheshm;
	unique_ptr<mmap_file_t>     _chunkmf[CHUNK_MAX];



	//singleton
	static unique_ptr<context_t> _context;
};
unique_ptr<context_t> context_t::_context;
static struct initialize_context_t{
	initialize_context_t()
	{
		context_t::get_context();
		log_d("");
	}
}init;
}}
#endif
