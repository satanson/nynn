#ifndef NYNN_MM_SUBGRAPH_ORGANIZATION_BY_SATANSON
#define NYNN_MM_SUBGRAPH_ORGANIZATION_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_subgraph_storage.h>
using namespace nynn::mm::common;
using namespace nynn::mm;

namespace nynn{namespace mm{
template <typename SubgraphStorageType>
class SubgraphOrganization;

template <typename SubgraphStorageType>
class SubgraphOrganization{
public:
	typedef typename SubgraphStorageType::Edge Edge;
	typedef typename SubgraphStorageType::Vertex Vertex;
	typedef typename SubgraphStorageType::Block Block;
	
    class EdgeIter{
		
	private:
		Block m_blk;
		uint32_t m_blk
		RWLock &m_lock;
	};


	explicit SubgraphOrganization(const string &basedir)
	try:
	m_storage(basedir),
	m_superblk(m_storage.getSuperBlock())
	{
	}catch(NynnException &err){
		throwNynnException("Fail to construct SubgraphOrganization object!");
	}	
	uint32_t getNumberOfEdges(uint32_t vtxno);
	void     getAllEdges(uint32_t vtxno,vector<Edge> &edges);
private:
	RWLock* getVertexRWLock(uint32_t vtxno)
	{

	}
	
	SubgraphStorageType m_storage;
	map<uint32_t,unique_ptr<RWLock> > m_vertexRWLockMap;
};
	
}}
#endif
