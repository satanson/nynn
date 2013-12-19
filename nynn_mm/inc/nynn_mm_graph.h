#include<nynn_mm_config.h>
#include<ProviderRPC.h>
using namespace std;
using namespace nynn::mm::common;
using namespace nynn::mm;
using namespace nynn::mm::rpc;

namespace nynn{namespace mm{

class Graph{
public:
	typedef string HostnameType;
	typedef uint32_t SubgraphKeyType;
	typedef map<HostnameType,std::shared_ptr<ProviderRPC> > Host2PrividerMap;
	typedef map<SubgraphKeyType,HostnameType> Subgraph2HostMap;
	typedef Host2PrividerMap::iterator Host2PrividerMapIterator;
	typedef Subgraph2HostMap::iterator Subgraph2HostMapIterator;

	Graph(HostnameType selfHost,uint32_t port,vector<HostnameType> hostnames)
		:m_selfHost(selfHost),m_port(port);
	{
		try{
			//construct ProviderRPC objects.
			for (int i=0;i<hostnames.size();i++){
				string &hostname=hostnames[i];
				m_host2ProviderMap[hostname].reset(new ProviderRPC(hostname,m_port));
			}
		}catch(NynnException &ex){
			throwNynnException("Failed to complete construction of Graph Object");
		}
	}

	void getLocalSubgraphKeys(vector<int8_t> keys)
	{
		keys.resize(0);
		Host2PrividerMapIterator it=m_host2ProviderMap.find(m_self);
		if (it!=m_host2ProviderMap.end())m_host2ProviderMap[m_self]->getSubgraphKeys(keys);
	}
	
	void setSubgraph2HostMap(const SubgraphKeyType &key, const HostnameType &hostname)
	{
		Synchronization s(&m_subgraph2HostMapMonitor);
		m_subgraph2HostMap[key]=hostname;
	}

	void setSubgraph2HostMap(const Subgraph2HostMap & s2hMap)
	{
		Synchronization s(&m_subgraph2HostMapMonitor);
		std::swap(m_subgraph2HostMap,s2hMap);
	}

	void refresh()
	{

	}
	
	bool lock(uint32_t vtxno)
	{

	}

	bool unlock(uint32_t vtxno)
	{

	}

	uint32_t getHeadBlkno(uint32_t vtxno)
	{

	}
	
	uint32_t getTailBlkno(uint32_t vtxno)
	{

	}
	
	bool readBlock(uint32_t vtxno,uint32_t blkno)
	{

	}

private:
	bool isUnKnown(uint32_t vtxno)
	{
		return m_subgraph2HostMap.find(SubgraphSet::VTXNO2SUBGRAPH(vtxno))
			   ==m_subgraph2HostMap.end();
	}
	
	bool isLocal(uint32_t vtxno)
	{
		return m_subgraph2HostMap[SubgraphSet::VTXNO2SUBGRAPH(vtxno)]==m_self;
	}

	bool isKnown(uint32_t vtxno) { return !isUnknown(vtxno); }
	bool isRemote(uint32_t vtxno) { return !isLocal(vtxno); }

	HostnameType     m_self;
	uint32_t 		 m_port;
	Host2PrividerMap m_host2ProviderMap;
	Subgraph2HostMap m_subgraph2HostMap;
	SubgraphCache    m_subgraphCache;
	Monitor 	     m_subgraph2HostMapMonitor;
};
}}
