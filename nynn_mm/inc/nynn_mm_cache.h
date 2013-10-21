#ifndef NYNN_MM_CACHE_BY_SATANSON
#define NYNN_MM_CACHE_BY_SATANSON
#include<nynn_mm_common.h>
using namespace nynn::mm::common;
namespace nynn{namespace mm{
template <
	size_t CACHE_BLOCK_MAX,
	size_t CACHE_BLOCK_SIZE,
	size_t CACHE_HEAD_MAX,
> class cache_t;

template <size_t CACHE_BLOCK_SIZE,size_t CACHE_BLOCK_MAX,size_t CACHE_HEAD_MAX>
class cache_t:public shm_allocator_t{
public:
	typedef cache_t<CACHE_BLOCK_SIZE,CACHE_BLOCK_MAX,CACHE_HEAD_MAX> type;
	typedef block_t<CACHE_BLOCK_SIZE> block_type;

	struct cache_entry_t{
		VertexNoType _source;
		ChunkBlockNoType _blkno;
		CacheBlockNoType _next;
		CacheBlockNoType _prev;
	};
	
	struct cache_head_t{
		CacheBlockNoType _next;
		CacheBlockNoType _prev;
		CacheListLenType _length;
	};

	static uint32_t hash(VertexNoType vertex,ChunkBlockNoType blkno)
	{
		uint32_t seed;
		vertex*=(blkno+1);
		seed=0xffffffff&&vertex;
		seed+=vertex>>32;
		return rand_r(&seed)%CACHE_HEAD_MAX;
	}
	
	block_type* read(VertexNoType vertex,ChunkBlockNoType blkno,block_type *blk)
	{

		size_t h=hash(vertex,blkno);
		size_t curr=find(h,vertex,blkno);
		
		if (curr==0)return 0;

		move_front(h,curr);

		block_type *block=reinterpret_cast<block_type>(&_tab)+curr;
		memcpy(blk,block,sizeof(block_type));
		return blk;
	}

	void write(VertexNoType vertex,ChunkBlockNoType blkno,block_type*blk)
	{

		size_t h=hash(vertex,blkno);
		size_t curr=find(h,vertex,blkno);

		if (curr=0){
			if (_tab._heads[h]._num==CACHE_BLOCK_MAX/CACHE_HEAD_MAX||_tab._free.next==0){
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
		
		block_type *block=reinterpret_cast<block_type>(&_tab)+curr;
		memcpy(block,blk,sizeof(block_type));
	}

	static type* get_cache(void*shm)
	{

		return new(shm)cache_t;
	}

	~cache_t()
	{
	}

private:
	cache_t()
	{
		memset(&_tab,0,sizeof(_tab));

		size_t start=(sizeof(_tab)+CACHE_BLOCK_SIZE-1)/CACHE_BLOCK_SIZE;

		for (size_t i=CACHE_BLOCK_MAX-1;i>=start;i--){
			_tab._entries[i]._next=_tab._free._next;
			_tab._free._next=i;
		}
	}

	cache_t(const cache_t&);
	cache_t& operator=(const cache_t&);

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
		assert(head._num<CACHE_BLOCK_MAX/CACHE_HEAD_MAX);
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
		cache_head_t  _heads[CACHE_HEAD_MAX];
		cache_head_t  _free;
		cache_entry_t _entries[CACHE_BLOCK_MAX];
	}_tab;
};

}}
#endif
