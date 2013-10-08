#ifndef NYNN_MM_LOCAL_BY_SATANSON
#define NYNN_MM_LOCAL_BY_SATANSON
#include<nynn_mm_common.h>
using namespace nynn::mm::common;
namespace nynn{namespace mm{namespace local{

template <
	size_t NENTRY=1<<8
> class  chunk_t;
template < size_t BLOCKSZ=1<<9 >struct block_t;
template < 
	size_t BLOCKSZ=(1<<9), 
	size_t NBLOCK=(1<<23), 
	size_t NVERTEX=(1<<23) 
> class subchunk_t;

struct vertex_t;
struct edge_t; 

struct vertex_t{
	uint64_t _source;
	size_t _nedge;
	size_t _blk;
};

struct edge_t{
	time_t _timestamp;
	uint64_t _sink;
	union{
		void* 	 _bval;
		uint64_t _uval;
		uint64_t  _ival;
		double   _fval;
		char 	 _cval[8];
	}_val;
};
template<size_t BLOCKSZ>
struct block_t{
	union{
		struct{
			size_t _prev;//prev block
			size_t _next;//next block
			time_t _mints;//min timestamp
			time_t _maxts;//max timestamp
			size_t _nedge;
		}_hdr;
		size_t _index[BLOCKSZ/sizeof(size_t)];
	}_blk;
};

template < size_t BLOCKSZ, size_t NBLOCK, size_t NVERTEX > 
class subchunk_t{
public:
	typedef subchunk_t<BLOCKSZ,NBLOCK,NVERTEX> subchunk;
	typedef block_t<BLOCKSZ> block;

	static subchunk* get_subchunk(void *shm)
	{
		subchunk* sch=static_cast<subchunk*>(shm);
		if (sch->_superblk==0){
			lock_t::get_lock(sch->_superblk._mutex,sizeof(lock_t));
		}
		sch->_superblk._refc++;
		return sch;
	}

	~subchunk_t(){
		lock_t *lck=lock_t::get_lock(_superblk._mutex,sizeof(lock_t));
		resetable_auto_ptr auto_lck(lck);
		raii_lock_t get(lck);
		_superblk._refc--;
		
		if (_superblk._refc>0)auto_lck.reset();
	}

	static void init(const string& path)
	{
		mmap_file_t mfile(path,NBLOCK*BLOCKSZ);
		void* addr=mfile.get_base();
		subchunk*sch=static_cast<subchunk*>(addr);

		sch->_superblk._refc=0;
		sch->_superblk._nblk=NBLOCK;
		sch->_superblk._blksz=BLOCKSZ;
		sch->_superblk._nvert=0;
		sch->_superblk._nused=(sizeof(_superblk)+BLOCKSZ-1)/BLOCKSZ*BLOCKSZ;
		sch->_superblk._nfree=0;
		sch->_superblk._head=0;
		sch->_superblk._top=0;

		log_i("nused=%d",sch->_superblk._nused);
		log_i("nfree=%d",sch->_superblk._nfree);
		for (size_t i=NBLOCK-1;i>=sch->_superblk._nused;i--)
			sch->release(i);
		
		mfile.sync(MS_SYNC|MS_INVALIDATE);
	}


	size_t require()
	{
		size_t blkno=0;
		lock_t * mutex=lock_t::get_lock(
				_superblk._mutex, sizeof(lock_t));	
		raii_lock_t get(mutex);
		size_t &head=_superblk._head;
		size_t &top=_superblk._top;
		//the freelist is empty,return 0 indicating failure of allocation.
		if (head==0) {
			blkno=0;
		//the head of freelist is empty.
	    } else if (top==0){
			block* blk=static_cast<block*>(&_superblk)+head;
			blkno=head;
			head=blk->_blk._index[top];
		//the freelist is not empty,the head of the freelist is not full.
		} else {
			block* blk=static_cast<block*>(&_superblk)+head;
			blkno=blk->_blk._index[top--];
		}
		_superblk._nfree--;
		return blkno;
	}

	void release(size_t blkno)
	{
		lock_t * mutex=lock_t::get_lock(
				_superblk._mutex, sizeof(lock_t));	

		raii_lock_t get(mutex);
		size_t &head=_superblk._head;
		size_t &top=_superblk._top;

		//the freelist is empty or head block of freelist is full before releasing.
		//make releasing block be head block of freelist.
		if (head==0 || top+1==BLOCKSZ/sizeof(size_t)){
			block *blk=reinterpret_cast<block*>(&_superblk)+blkno;
			top=0;
			blk->_blk._index[top]=head;
			head=blkno;
		//the freelist is not empty and the head block of the freelist is not full.
		}else {
			block *blk=reinterpret_cast<block*>(&_superblk)+head;
			blk->_blk._index[++top]=blkno;
		}

		_superblk._nfree++;
	}
	
private:

	struct{
		size_t _refc;
		size_t _nblk;  //total number of blocks in chunk.
		size_t _blksz; //block size in bytes.

		size_t _nfree; //number of available blocks.
		size_t _nused; //number of used blocks.
		size_t _nvert;


		size_t _head;//the index of first head blocks;
		size_t _top;

		char   _mutex[sizeof(lock_t)];
		vertex_t _vertices[NVERTEX];
	}_superblk;
};

template <size_t NENTRY>
class chunk_t{
public:
	enum{
		PATH_SIZE=64,
		CE_FLAG_PRESENT=1,
		CE_FLAG_WHERE=2,
		DUMMY
	};
	typedef chunk_t<NENTRY> chunk;
	struct chunk_entry_t{
		char _path[PATH_SIZE];
		uint8_t  _flag;	
		uint64_t _minvertex;
		uint64_t _maxvertex;
		uint32_t _where;
	};
private:
	struct{
		int _refc;
		char _mutex[sizeof(mutex)];
		chunk_entry_t _entries[NENTRY];
	}_tab;
};

}}}
#endif
