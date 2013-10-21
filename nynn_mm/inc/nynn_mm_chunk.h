#ifndef NYNN_MM_CHUNK_BY_SATANSON
#define NYNN_MM_CHUNK_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_config.h>
using namespace nynn::mm::common;
namespace nynn{namespace mm{

template < size_t BLOCK_SIZE >struct block_t;

template < 
	size_t BLOCK_SIZE, 
	size_t CHUNK_BLOCK_MAX,
	size_t CHUNK_VERTEX_MAX
> class chunk_t;

struct vertex_t;
struct edge_t; 

struct vertex_t{
	VertexNoType     _source;
	EdgeNumType      _nedge;
	ChunkBlockNoType _blkno;
};

struct edge_t{
	time_t _timestamp;
	VertexNoType _sink;
	union{
		void* 	 _bval;
		uint64_t _uval;
		uint64_t  _ival;
		double   _fval;
		char 	 _cval[8];
	}_val;
};
template<size_t BLOCK_SIZE>
struct block_t{
	union{
		struct{
			ChunkBlockNoType _prev;//prev block
			ChunkBlockNoType _next;//next block
			time_t _mints;//min timestamp
			time_t _maxts;//max timestamp
			EdgeNumType _nedge;
		}_hdr;
		ChunkBlockNoType _index[BLOCK_SIZE/sizeof(ChunkBlockNoType)];
	}_blk;
};

template < size_t BLOCK_SIZE, size_t CHUNK_BLOCK_MAX, size_t CHUNK_VERTEX_MAX > 
class chunk_t{
public:
	typedef chunk_t<BLOCK_SIZE,CHUNK_BLOCK_MAX,CHUNK_VERTEX_MAX> chunk_type;
	typedef block_t<BLOCK_SIZE> block;

	static chunk_type* get_chunk(void *shm)
	{
		chunk_type* chk=static_cast<chunk_type*>(shm);
		return chk;
	}

	~chunk_t(){}

	static void init(const string& path)
	{
		unique_ptr<mmap_file_t> chunkmf(new mmap_file_t(path,CHUNK_BLOCK_MAX*BLOCK_SIZE));

		chunk_type*chk=static_cast<chunk_type*>(chunkmf->get_base());

		chk->_superblk._nblk=CHUNK_BLOCK_MAX;
		chk->_superblk._blksz=BLOCK_SIZE;
		chk->_superblk._nvert=0;
		chk->_superblk._nused=(sizeof(_superblk)+BLOCK_SIZE-1)/BLOCK_SIZE*BLOCK_SIZE;
		chk->_superblk._nfree=0;
		chk->_superblk._head=0;
		chk->_superblk._top=0;

		log_i("nused=%d",chk->_superblk._nused);
		log_i("nfree=%d",chk->_superblk._nfree);
		for (size_t i=CHUNK_BLOCK_MAX-1;i>=chk->_superblk._nused;i--)
			chk->release(i);
	}


	size_t require()
	{
		size_t blkno=0;

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
		size_t &head=_superblk._head;
		size_t &top=_superblk._top;

		//the freelist is empty or head block of freelist is full before releasing.
		//make releasing block be head block of freelist.
		if (head==0 || top+1==BLOCK_SIZE/sizeof(size_t)){
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
	size_t get_minvertex()const{return _superblk._vertices[0]._source;}
	size_t get_maxvertex()const{return _superblk._vertices[_superblk._nvert-1]._source;}
	size_t get_vertexnum()const{return _superblk._nvert;}
	
private:

	struct{
		size_t _nblk;  //total number of blocks in chunk.
		size_t _blksz; //block size in bytes.

		size_t _nfree; //number of available blocks.
		size_t _nused; //number of used blocks.
		size_t _nvert;


		size_t _head;//the index of first head blocks;
		size_t _top;

		vertex_t _vertices[CHUNK_VERTEX_MAX];
	}_superblk;
};


}}
#endif
