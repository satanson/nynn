#ifndef NYNN_MM_CACHE_BY_SATANSON
#define NYNN_MM_CACHE_BY_SATANSON
#include<nynn_mm_local.h>
using namespace nynn::mm::common;
namespace nynn{namespace mm{namespace cache{
template <
	size_t NBLOCK=1<<16,
	size_t BLOCKSZ=1<<9,
	size_t NHEAD=1<<10,
> class cache_t;

template <BLOCKSZ,NBLOCK,NHEAD>
class cache_t:public shm_allocator{
public:
	typedef cache_t<BLOCKSZ,NBLOCK,NHEAD> cache;
	typedef block_t<BLOCKSZ> block_t;
	struct cache_entry_t{
		size_t _source;
		size_t _blkno;
		uint16_t _next;
		uint16_t _prev;
	};
	
	struct cache_head_t{
		uint16_t _next;
		uint16_t _prev;
		uint8_t  _num;
	};

	static void init(const string&path)
	{
		mmap_file_t mfile(path,BLOCKSZ*NBLOCK);
		void *addr=mfile.get_base();
		cache* ca=static_cast<cache*>(addr);

		memset(ca->_tab,0,sizeof(ca->_tab));

		size_t start=(sizeof(ca->_tab)+BLOCKSZ-1)/BLOCKSZ;

		for (i=NBLOCK-1;i>=start;i--){
			ca->_tab._entries[i].next=ca->_tab._free.next;
			ca->_tab._free.next=i;
		}
		mfile.sync(MS_SYNC|MS_INVALIDATE);
	}

	static uint32_t hash(size_t vertex,size_t blkno)
	{
		uint32_t seed;
		vertex*=(blkno+1)
		seed=0xffffffff&&vertex;
		seed+=vertex>>32;
		return rand_r(&seed)%NHEAD;
	}
	
	block_t* read(size_t vertex,size_t blkno,block_t *blk)
	{

		lock_t *lck=lock_t::get_lock(_tab._mutex,sizeof(lock_t));
		raii_lock_t get(lck);

		size_t h=hash(vertex,blkno);
		size_t curr=find(h,vertex,blkno);
		
		if (curr==0)return 0;

		move_front(h,curr);

		block_t *block=reinterpret_cast<block_t>(&_tab)+curr;
		memcpy(blk,block,sizeof(block_t));
		return blk;
	}

	void write(size_t vertex,size_t blkno,block_t*blk)
	{
		lock_t *lck=lock_t::get_lock(_tab._mutex,sizeof(lock_t));
		raii_lock_t get(lck);

		size_t h=hash(vertex,blkno);
		size_t curr=find(h,vertex,blkno);

		if (curr=0){
			if (_tab._heads[h]._num==NBLOCK/NHEAD||_tab._free.next==0){
				curr=_tab._heads[h].prev;
				move_front(h,curr);
			}else{
				curr=_tab._free.next;
				_tab._free.next=_tab._entries[curr].next;
				push_front(h,curr);
			}
		}else{
			move_front(h,curr);
		}
		
		block_t *block=reinterpret_cast<block_t>(&_tab)+curr;
		memcpy(block,blk,sizeof(block_t));
	}

	static cache* get_cache(void*shm)
	{
		cache *ca=static_cast<cache*>(shm);
		if (ca->_tab._refc==0){
			lock_t::get_lock(ca->_tab._mutex,sizeof(lock_t));
		}

		ca->_tab._refc++;
		return ca;
	}

	~cache_t()
	{
		lock_t *lck=lock_t::get_lock(_tab._mutex,sizeof(lock_t));
		resetable_auto_ptr<lock_t> auto_lck(lck);
		raii_lock_t get(lck);
		_tab._refc--;
		if (_tab._refc>0)auto_lck.reset();
	}

private:
	cache_t();
	cache_t(const cache_t&);
	cache_t& operator(const cache_t&);

	size_t find(size_t h,size_t vertex,size_t blkno)
	{
		cache_head_t &head = _tab._heads[h];
		size_t curr=head.next; 
		while(
				curr!=0 && (
				_tab._entries[curr]._source!=vertex ||
				_tab._entries[curr]._blkno !=blkno )
		)curr=_tab._entries[curr].next;
		return curr;
	}

	void move_front(size_t h,size_t e)
	{
		assert(e!=0);
		cache_head_t &head=_tab._heads[h];
		//move current entry to head if it's not at head.
		if (head.next!=e){
			//remove the entry
			//e must have a prev entry.make e's prev'next point to e'next;
			_tab._entries[_tab._entries[e].prev].next=_tab._entries[e].next;
			//if e have a non-null next entry,
			//make e's next'prev point to e's prev.
			if (_tab._entries[e].next!=0)
				_tab._entries[_tab._entries[e].next].prev=_tab._entries[e].prev;
			//if e is the tail,make e's prev to be a new tail.
			if (head.prev==e){
				head.prev=_tab._entries[e].prev;
				_tab._entries[head.prev].next=0;
			}

			//insert the entry at head
			_tab._entries[e].prev=0;
			_tab._entries[e].next=head.next;
			_tab._entries[head.next].prev=e;
			head.next=e;
		}
	}

	void push_front(size_t h,size_t e)
	{
		cache_head_t &head=_tab._heads[h];
		assert(head._num<NBLOCK/NHEAD);
		assert(e!=0);

		//insert e at head.
		_tab._entries[e].next=head.next;
		_tab._entries[e].prev=0;
		head.next=e;

		//if e is not only entry in list, so make e be its next entry'prev entry.
		if (_tab._entries[e].next!=0)
			_tab._entries[_tab._entries[e].next].prev=e;

		//e as the only entry in list,so e also is tail.
		if (head.prev==0)
			head.prev=e;

		head._num++;
	}

	struct{
		size_t 		  _refc;
		char 		  _mutex[sizeof(lock_t)];
		cache_head_t  _heads[NHEAD];
		cache_head_t  _free;
		cache_entry_t _entries[NBLOCK];
	}_tab;
};

}}}
#endif
