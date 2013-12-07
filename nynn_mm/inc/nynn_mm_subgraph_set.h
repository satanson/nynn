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
			string hostname;
			uint32_t m_myhost=gethostaddr(hostname);
			log_i("hostname=%s",hostname.c_str());
			memset(m_subgraphSites,0,sizeof(my_hosts));

			//initialize Subgraph Storage
			glob_t g;
			g.gl_offs=0;
			string subgraphPathPattern=basedir+"/subgraph*";
			int retcode=glob(subgraphPattern.c_str(),0,0,&g);
			if (retcode!=0 && retcode!=GLOB_NOMATCH) {
				string errinfo=string("Fail to invoke glob!")+"("+strerr(errno)+")";
				throwNynnException(errinfo.c_str());
			}
			assert(g.gl_pathc<=SUBGRAPH_ENTRY_NUM);
			for (uint32_t  i=0;i<g.gl_pathc;i++){
				const char* path=g.gl_pathv[i];
				const char* strSubgraphMinVtxno=path+strlen(path)-strlen("0xabcd0000");
				uint32_t subgraphMinVtxno=strtoul(strSubgraphMinVtxno,NULL,16);
				assert(subgraphMinVtxno%VERTEX_INTERVAL_WIDTH==0);
				m_subgraphSites[lambda(subgraphMinVtxno)]=m_myhost;
				m_subgraphs[lambda(subgraphMinVtxno)].reset(new SubgraphStorageT(path));
			}

			//initialize Subgraph Cache 
			m_cache.reset(new SubgraphCacheT());
		}catch(NynnException &ex){
			throwNynnException("Fail to create Graph object");
		}
	}

	static uint32_t const IS_WRITABLE=1;
	static uint32_t const IS_READABLE=0;
	static uint32_t const IS_NONBLOCKING=2;
	static uint32_t const IS_BLOCKING=0;
	
	
	void setSubgraph(const string basedir)
	{
		ExclusiveSynchronization es(&m_subgraphMapRWLock);

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

	uint32_t getFirstBlkno(uint32_t vtxno)
	{
		return getSubgraph(vtxno)->getFristBlkno(vtxno);	
	}

	uint32_t setFirstBlkno(uint32_t vtxno, uint32_t blkno)
	{
		getSubgraph(vtxno)->setFirstBlkno(vtxno,blkno);
	}

	bool containVertexno(uint32_t vtxno)
	{
		shared_ptr<SubgrpahStorageT> subgraph=getSubgraph(vtxno);
		return subgraph.get()!=NULL?subgraph->containVertexno(vtxno):false;
	}
	void read(uint32_t vtxno,uint32_t blkno,Block *blk)	
	{
		getSubgraph(vtxno)->readBlock(blkno,blk);	
	}
	uint32_t getNumberOfEdges(uint32_t vtxno)
	{
		return getSubgraph(vtxno)->getNumberOfEdges(vtxno);
	}		
	void setNumberOfEdges(uint32_t vtxno,uint32_t nedges)
	{
		getSubgraph(vtxno)->setNumberOfEdges(nedges);
	}
	void write(uint32_t vtxno,uint32_t blkno,Block *blk) 
	{
		getSubgraph(vtxno).writeBlock(blkno,blk);
	}
	
	void insert(uint32_t vtxno,Block* blk)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);
		Block::BlockHeader *header=blk->getBlock();
		Content *content=blk;
		subgraph->resize(vtxno,subgraph->size(vtxno)+content->size());
		// Exception::not enough space.	
		uint32_t blkno=subgraph->require();
		if (blkno==INVALID_BLOCKNO){
			throwNynnException("Not enough space on subgraph");
		}

		uint32_t firstBlkno=subgraph->getFirstBlkno(vtxno);
		// insert first block.
		if (firstBlkno==INVALID_BLOCKNO){
			header->setPrev(INVALID_BLOCKNO);
			header->setNext(INVALID_BLOCKNO);
			subgraph->setFirstBlkno(blkno);
		}

		// 
		uint32_t blkno=firstBlkno;
		Block::BlockHeader *prevHeader=NULL;
		while(true){
			currHeader=getBlock(blkno)->getHeader();
			if(currHeader->isOlderThan(*header))break;
		}
	}

	void remove(uint32_t vtxno,uint32_t blkno)
	{
		shared_ptr<SubgraphStorageT> &subgraph=getSubgraph(vtxno);

		uint32_t firstBlkno=subgraph->getFirstBlkno(vtxno);

		if (firstBlkno!=INVALID_BLOCKNO)return;
		//fatal error!!! Block::BlockHeader *=subgraph->getBlock(blkno)->getHeader();
		Block *blk=subgraph->getBlock(blkno);
		Block::BlockHeader header=*blk->getHeader();
		Content *content=blk;
		subgraph->resize(vtxno,subgraph->size(vtxno)-content->size());
		//remove first block.
		if (firstBlkno==blkno) {
			subgraph->setFirstBlkno(header.getNext());
			subgraph->release(blkno);
		//remove other block instead of first one. 
		}else{
			uint32_t prevBlkno=header.getPrev();
			uint32_t nextBlkno=header.getNext();

			subgraph->getBlock(header.getPrev())->getHeader()->setNext(nextBlkno);
			subgraph->getBlock(header.getNext())->getHeader()->setPrev(prevBlkno);
			subgraph->release(blkno);
		}
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
