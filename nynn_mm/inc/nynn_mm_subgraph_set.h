#ifndef NYNN_MM_SUBGRAPH_SET_BY_SATANSON
#define NYNN_MM_SUBGRAPH_SET_BY_SATANSON
#include <nynn_mm_common.h>
#include <nynn_mm_subgraph_storage.h>
using namespace nynn::mm::common;
using namespace nynn::mm;
namespacespace nynn{namespace mm{
template<
	class SubgraphStorageT,
	class Block,
	class BlockContent,
	uint32_t VERTEX_RWLOCK_NUM
>class SubgraphSetType;

template<
	class SubgraphStorageT,
	class Block,
	class BlockContent,
	uint32_t VERTEX_RWLOCK_NUM
>class SubgraphSetType{
public:
	//very important const parameter
	static uint32_t const LOG2_VERTEX_INTERVAL_WIDTH=SubgraphStorageT::LOG2_VERTEX_INTERVAL_WIDTH;
	static uint32_t const VERTEX_INTERVAL_WIDTH=(1<<LOG2_VERTEX_INTERVAL_WIDTH); 
	static uint32_t const SUBGRAPH_ENTRY_NUM=1<<(32-LOG2_VERTEX_INTERVAL_WIDTH);
	
	typedef map<uint32_t,shared_ptr<SubgraphStorageT> > SubgraphMap;
	typedef SubgraphMap::iterator SubgraphMapIterator;

	SubgraphSetType(const string &graphBasedir):m_graphBasedir(graphBasedir)
	{
		try{
			glob_t g;
			g.gl_offs=0;
			string subgraphPathPattern=basedir+"/subgraph0x????????";
			int retcode=glob(subgraphPattern.c_str(),0,0,&g);
			if (retcode!=0 && retcode!=GLOB_NOMATCH) {
				string errinfo=string("Fail to invoke glob!")+"("+strerr(errno)+")";
				throwNynnException(errinfo.c_str());
			}
			for (uint32_t  i=0;i<g.gl_pathc;i++) setSubgraph(g.gl_pathv[i]);
		}catch(NynnException &ex){
			throwNynnException("Fail to create Graph object");
		}
	}

	static uint32_t const IS_WRITABLE=1;
	static uint32_t const IS_READABLE=0;
	static uint32_t const IS_NONBLOCKING=2;
	static uint32_t const IS_BLOCKING=0;
	
	
	void setSubgraph(const string &basedir)
	{
		ExclusiveSynchronization es(&m_subgraphMapRWLock);
		const char* path=basedir.c_str();
		const char* strSubgraphMinVtxno=path+strlen(path)-strlen("0xabcd0000");
		uint32_t subgraphMinVtxno=strtoul(strSubgraphMinVtxno,NULL,16);
		m_subgraphMap[subgraphMinVtxno].reset(new SubgraphStorageT(basedir));
	}
	
	shared_ptr<SubgraphStorageT>& getSubgraph(uint32_t vtxno)
	{
		SharedSynchronization ss(&m_subgraphMapRWLock);
		
		SubgraphMapIterator it=m_subgraphMap.find(VTXNO2SUBGRAPH(vtxno));
		if (it==m_subgraphMap.end()){
			return shared_ptr<SubgraphStorageT>(NULL);
		}
		return *it;
	}
	//bit0: 0(read).1(read&write)
	//bit1: 0(blocking).1(nonblocking).
	bool lock(uint32_t vtxno,uint32_t flag)
	{

		pthread_rwlock_t *rwlock=m_vertexRWLocks[VTXNO2RWLOCK].get();

		if (flag&IS_WRITABLE==IS_WRITABLE){
			if (flag&IS_NONBLOCKING==IS_NONBLOCKING) {
				if (pthread_rwlock_trywrlock(rwlock)!=0)return false;
			}else{
				pthread_rwlock_wrlock(rwlock);
			}
		}else{
			if (flag&IS_NONBLOCKING==IS_NONBLOCKING) {
				if (pthread_rwlock_tryrdlock(rwlock)!=0)return false;
			}else{
				pthread_rwlock_rdlock(rwlock);
			}
		}	

		return true;
	}


	void unlock(uint32_t vtxno)
	{
		pthread_rwlock_t *rwlock=m_vertexRWLocks[VTXNO2RWLOCK].get();
		if (pthread_rwlock_unlock(rwlock)!=0){
			throwNynnException("Fail to unlock vertex!");
		}
	}

	uint32_t getHeadBlkno(uint32_t vtxno)
	{
		return getSubgraph(vtxno)->getHeadBlkno(vtxno);	
	}

	uint32_t getTailBlkno(uint32_t vtxno)
	{
		return getSubgraph(vtxno)->getTailBlkno(vtxno);	
	}

	void read(uint32_t vtxno,uint32_t blkno,Block *blk)	
	{
		getSubgraph(vtxno)->readBlock(blkno,blk);
	}

	uint32_t getSize(uint32_t vtxno)
	{
		return getSubgraph(vtxno)->getVertex(vtxno)->size();
	}		

	uint32_t insertPrev(uint32_t vtxno,uint32_t nextBlkno, Block* blk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex *vtx=subgraph->getVertex(vtxno);

		uint32_t headBlkno=vtx->getHeadBlkno();
		if (nextBlkno==headBlkno){
			return unshift(vtxno,blk);
		}else if (nextBlkno==INVALID_BLOCKNO){
			return push(vtxno,blk);
		}else{
			Content *content=*blk;
			vtx->resize(vtx->size()+blk->size());
			uint32_t blkno=subgraph->require();
			if (blkno==INVALID_BLOCKNO)return INVALID_BLOCKNO;

			Block *nextBlk=subgraph->getBlock(nextBlkno);
			uint32_t prevBlkno=nextBlk->getPrev();
			nextBlk->getHeader()->setPrev(blkno);

			Block *prevBlk=subgraph->getBlock(prevBlkno);
			prevBlk->getHeader()->setNext(blkno);
			
			blk->getHeader()->setNext(nextBlkno);
			blk->getHeader()->setPrev(prevBlkno);
			subgraph->writeBlock(blkno,blk);
			return blkno;
		}
	}

	uint32_t insertNext(uint32_t vtxno,uint32_t prevBlkno,Block* blk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex *vtx=subgraph->getVertex(vtxno);

		uint32_t tailBlkno=vtx->getTailBlkno();
		if (prevBlkno==tailBlkno){
			return push(vtxno,blk);
		}else if (prevBlkno==INVALID_BLOCKNO){
			return unshift(vtxno,blk);
		}else{
			Content *content=*blk;
			vtx->resize(vtx->size()+blk->size());
			uint32_t blkno=subgraph->require();
			if (blkno==INVALID_BLOCKNO)return INVALID_BLOCKNO;

			Block *prevBlk=subgraph->getBlock(prevBlkno);
			uint32_t nextBlkno=prevBlk->getNext();
			prevBlk->getHeader()->setNext(blkno);

			Block *nextBlk=subgraph->getBlock(nextBlkno);
			nextBlk->getHeader()->setPrev(blkno);
			
			blk->getHeader()->setNext(nextBlkno);
			blk->getHeader()->setPrev(prevBlkno);
			subgraph->writeBlock(blkno,blk);
			return blkno;
		}
	}

	void remove(uint32_t vtxno,uint32_t blkno)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		
		Vertex *vtx=subgraph->getVertex(vtxno);

		uint32_t headBlkno=vtx->getHeadBlkno();
		uint32_t tailBlkno=vtx->getTailBlkno();

	    //if Vertex has no blocks.
		//tailBlkno==INVALID_BLOCKNO also is TRUE
		if (headBlkno==INVALID_BLOCKNO)return;

		//remove head block.
		if (headBlkno==blkno){
			shift(vtxno);
		//remove tail block.
		}else if (TailBlkno==blkno){
			pop(vtxno);
		//remove inner block that has prev or next block.
		}else{
			Block *blk=subgraph->getBlock(blkno);
			Content *content=*blk;
			vtx->resize(vtx->size()-content->size());

			uint32_t prevBlkno=blk->getHeader()->getPrev();
			uint32_t nextBlkno=blk->getHeader()->getNext();
			subgraph->getBlock(prevBlkno)->setNext(nextBlkno);
			subgraph->getBlock(nextBlkno)->setPrev(prevBlkno);
		}
	}
	
	uint32_t unshift(uint32_t vtxno,Block*newHeadBlk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex* vtx=subgraph->getVertex(vtxno);
		Content *newHeadBlkContent=*newHeadBlk;
	
		vtx->resize(vtx->size()+newHeadBlkContent->size());
		uint32_t newHeadBlkno=subgraph->require();
		if (newHeadBlkno==INVALID_BLOCKNO)return INVALID_BLOCKNO;
		uint32_t oldHeadBlkno=vtx->getHeadBlkno();

		if (oldHeadBlkno != INVALID_BLOCKNO){
			Block* oldHeadBlk=subgraph->getBlock(oldHeadBlkno);
			vtx->setHeadBlkno(newHeadBlkno);
			newHeadBlk->getHeader()->setNext(oldHeadBlkno);
			newHeadBlk->getHeader()->setPrev(INVALID_BLOCKNO);
			oldHeadBlk->getHeader()->setPrev(newHeadBlkno);
		}else{
			vtx->setHeadBlkno(newHeadBlkno);
			vtx->setTailBlkno(newHeadBlkno);
			newHeadBlk->getHeader()->setNext(INVALID_BLOCKNO);
			newHeadBlk->getHeader()->setPrev(INVALID_BLOCKNO);
		}
		subgraph->writeBlock(newHeadBlkno,newHeadBlk);
		return newHeadBlkno;
	}

	void shift(uint32_t vtxno)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex *vtx=subgraph->getVertex(vtxno);
		uint32_t oldHeadBlkno=vtx->getHeadBlkno();

		if (oldHeadBlkno == INVALID_BLOCKNO)return;

		Block *oldHeadBlk=subgraph->getBlock(oldHeadBlkno);
		uint32_t newHeadBlkno=oldHeadBlk->getHeader()->getNext();
		Content *oldHeadBlkContent=*oldHeadBlk;
		vtx->resize(vtx->size()-oldHeadBlkContent->size());


		if (newHeadBlkno != INVALID_BLOCKNO){
			Block *newHeadBlk=subgraph->getBlock(newHeadBlkno);
			vtx->setHeadBlkno(newHeadBlkno);
			newHeadBlk->getHeader()->setPrev(INVALID_BLOCKNO);
		}else{
			vtx->setHeadBlkno(INVALID_BLOCKNO);
			vtx->setTailBlkno(INVALID_BLOCKNO);
		}
		release(oldHeadBlkno);
	}

	uint32_t push(uint32_t vtxno,Block*newTailBlk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex* vtx=subgraph->getVertex(vtxno);
		Content *newTailBlkContent=*newTailBlk;
	
		vtx->resize(vtx->size()+newTailBlkContent->size());
		uint32_t newTailBlkno=subgraph->require();
		if (newTailBlkno==INVALID_BLOCKNO) return INVALID_BLOCKNO;
		uint32_t oldTailBlkno=vtx->getTailBlkno();

		if (oldTailBlkno != INVALID_BLOCKNO){
			Block* oldTailBlk=subgraph->getBlock(oldTailBlkno);
			vtx->setTailBlkno(newTailBlkno);
			newTailBlk->getHeader()->setPrev(oldTailBlkno);
			newTailBlk->getHeader()->setNext(INVALID_BLOCKNO);
			oldTailBlk->getHeader()->setNext(newTailBlkno);
		}else{
			vtx->setTailBlkno(newTailBlkno);
			vtx->setTailBlkno(newTailBlkno);
			newTailBlk->getHeader()->setPrev(INVALID_BLOCKNO);
			newTailBlk->getHeader()->setNext(INVALID_BLOCKNO);
		}
		subgraph->writeBlock(newTailBlkno,newTailBlk);
		return INVALID_BLOCKNO;
	}

	void pop(uint32_t vtxno,uint32_t blkno)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Vertex *vtx=subgraph->getVertex(vtxno);
		uint32_t oldTailBlkno=vtx->getTailBlkno();

		if (oldTailBlkno == INVALID_BLOCKNO)return;

		Block *oldTailBlk=subgraph->getBlock(oldTailBlkno);
		uint32_t newTailBlkno=oldTailBlk->getHeader()->getPrev();
		Content *oldTailBlkContent=*oldTailBlk;
		vtx->resize(vtx->size()-oldTailBlkContent->size());


		if (newTailBlkno != INVALID_BLOCKNO){
			Block *newTailBlk=subgraph->getBlock(newTailBlkno);
			vtx->setTailBlkno(newTailBlkno);
			newTailBlk->getHeader()->setNext(INVALID_BLOCKNO);
		}else{
			vtx->setTailBlkno(INVALID_BLOCKNO);
			vtx->setTailBlkno(INVALID_BLOCKNO);
		}
		release(oldTailBlkno);
	}

private:
	static uint32_t VTXNO2SUBGRAPH(uint32_t vtxno) { return vtxno>>LOG2_VERTEX_INTERVAL_WIDTH; }
	static uint32_t VTXNO2RWLOCK(uint32_t vtxno){ return vtxno%VERTEX_RWLOCK_NUM;}
	
	string m_graphBasedir;
	RWLock m_vertexRWLocks[VERTEX_RWLOCK_NUM];
	RWLock m_subgraphMapRWLock;
	SubgraphMap m_subgraphMap;
};

}}
#endif
