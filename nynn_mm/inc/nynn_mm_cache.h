#ifndef NYNN_MM_CACHE_BY_SATANSON
#define NYNN_MM_CACHE_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_subgraph_storage.h>

using namespace nynn::mm::common;
namespace nynn{namespace mm{
template <
	typename SubgraphStorageType,
	uint32_t CACHE_BLOCK_NUM,
	uint32_t CACHE_HEAD_NUM
> class SubgraphCache;

template <
	typename SubgraphStorageType,
	uint32_t CACHE_BLOCK_NUM,
	uint32_t CACHE_HEAD_NUM
>
class SubgraphCache{
public:
	typedef typename SubgraphStorageType::Edge Edge;
	typedef typename SubgraphStorageType::Vertex Vertex;
	typedef typename SubgraphStorageType::Block  Block;	
	typedef 

	struct SubgraphCacheEntry{
		uint64_t m_source;
		uint32_t m_blkno;
		uint32_t m_next;
		uint32_t m_prev;
	};
	
	struct SubgraphCacheHead{
		uint32_t m_next;
		uint32_t m_prev;
		uint32_t m_size;
	};

	static uint32_t hash(uint64_t vertex,uint32_t m_blkno)
	{
		vertex*=(m_blkno+1);
		uint32_t seed=0xffffffff&&vertex;
		seed+=vertex>>32;
		return rand_r(&seed)%CACHE_HEAD_NUM;
	}
	
	Block* read(uint64_t vertex,uint32_t m_blkno,Block *blk)
	{

		size_t h=hash(vertex,m_blkno);
		size_t curr=find(h,vertex,m_blkno);
		
		if (curr==0)return 0;

		move_front(h,curr);

		Block *block=reinterpret_cast<Block>(&tab)+curr;
		memcpy(blk,block,sizeof(Block));
		return blk;
	}

	void write(uint64_t vertex,uint32_t m_blkno,Block*blk)
	{

		size_t h=hash(vertex,m_blkno);
		size_t curr=find(h,vertex,m_blkno);

		if (curr=0){
			if (m_heads[h].num==CACHE_BLOCK_NUM/CACHE_HEAD_NUM||m_free.m_next==0){
				curr=m_heads[h].m_prev;
				move_front(h,curr);
			}else{
				curr=m_free.m_next;
				m_free.m_next=m_entries[curr].m_next;
				push_front(h,curr);
			}
		}else{
			move_front(h,curr);
		}
		
		Block *block=reinterpret_cast<Block>(&tab)+curr;
		memcpy(block,blk,sizeof(Block));
	}

	~SubgraphCache()
	{
	}

private:
	SubgraphCache()
	{
		memset(&tab,0,sizeof(tab));

		size_t start=(sizeof(tab)+CACHE_BLOCK_SIZE-1)/CACHE_BLOCK_SIZE;

		for (size_t i=CACHE_BLOCK_NUM-1;i>=start;i--){
			m_entries[i].m_next=m_free.m_next;
			m_free.m_next=i;
		}
	}

	SubgraphCache(const SubgraphCache&);
	SubgraphCache& operator=(const SubgraphCache&);

	size_t find(size_t h,uint64_t vertex,uint32_t m_blkno)
	{
		SubgraphCacheHead &head = m_heads[h];
		size_t curr=head.m_next; 
		while(
				curr!=0 && (
				m_entries[curr].m_source!=vertex ||
				m_entries[curr].m_blkno !=m_blkno )
		)curr=m_entries[curr].m_next;
		return curr;
	}

	void move_front(size_t h,size_t e)
	{
		assert(e!=0);
		SubgraphCacheHead &head=m_heads[h];
		//move current entry to head if it's not at head.
		if (head.m_next!=e){
			//remove the entry
			//e must have a m_prev entry.make e's m_prev'm_next point to e'm_next;
			m_entries[m_entries[e].m_prev].m_next=m_entries[e].m_next;
			//if e have a non-null m_next entry,
			//make e's m_next'm_prev point to e's m_prev.
			if (m_entries[e].m_next!=0)
				m_entries[m_entries[e].m_next].m_prev=m_entries[e].m_prev;
			//if e is the tail,make e's m_prev to be a new tail.
			if (head.m_prev==e){
				head.m_prev=m_entries[e].m_prev;
				m_entries[head.m_prev].m_next=0;
			}

			//insert the entry at head
			m_entries[e].m_prev=0;
			m_entries[e].m_next=head.m_next;
			m_entries[head.m_next].m_prev=e;
			head.m_next=e;
		}
	}

	void push_front(size_t h,size_t e)
	{
		SubgraphCacheHead &head=m_heads[h];
		assert(head.num<CACHE_BLOCK_NUM/CACHE_HEAD_NUM);
		assert(e!=0);

		//insert e at head.
		m_entries[e].m_next=head.m_next;
		m_entries[e].m_prev=0;
		head.m_next=e;

		//if e is not only entry in list, so make e be its m_next entry'm_prev entry.
		if (m_entries[e].m_next!=0)
			m_entries[m_entries[e].m_next].m_prev=e;

		//e as the only entry in list,so e also is tail.
		if (head.m_prev==0)
			head.m_prev=e;

		head.num++;
	}

	SubgraphCacheHead  m_heads[CACHE_HEAD_NUM];
	SubgraphCacheHead  m_freeList;
	SubgraphCacheEntry m_entries[CACHE_BLOCK_NUM];
};

}}
#endif
