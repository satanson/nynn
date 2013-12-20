#include<nynn_mm_config.h>
#include<ProviderRPC.h>
using namespace std;
using namespace nynn::mm::common;
using namespace nynn::mm;
using namespace nynn::mm::rpc;

namespace nynn{namespace mm{

class Graph{
public:
	typedef string string;
	typedef uint32_t uint32_t;
	typedef map<string,std::shared_ptr<ProviderRPC> > Host2PrividerMap;
	typedef map<uint32_t,string> Subgraph2HostMap;
	typedef Host2PrividerMap::iterator Host2PrividerMapIterator;
	typedef Subgraph2HostMap::iterator Subgraph2HostMapIterator;

	Graph(string selfHost,uint32_t port,vector<string> hostnames)
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
	
	void setSubgraph2HostMap(const uint32_t &key, const string &hostname)
	{
		ExclusiveSynchronization es(&m_subgraph2HostMapRWLock);
		m_subgraph2HostMap[key]=hostname;
	}

	void setSubgraph2HostMap(const Subgraph2HostMap & s2hMap)
	{
		ExclusiveSynchronization s(&m_subgraph2HostMapRWLock);
		std::swap(m_subgraph2HostMap,s2hMap);
	}

	void refresh()
	{

	}
	
	static uint32_t const IS_READABLE=SubgraphSet::IS_READABLE;
	static uint32_t const IS_NONBLOCKING=SubgraphSet::IS_NONBLOCKING;
	static uint32_t const IS_BLOCKING=SubgraphSet::IS_BLOCKING;

	bool lock(uint32_t vtxno,bool nonblocking)
	{
		string hostname=getHostnameTS(vtxno);
		uint32_t flag=(nonblocking?IS_NONBLOCKING:IS_BLOCKING)|IS_READABLE;

		if (hostname!="") return getProvider(hostname)->lock(vtxno,flag);

		refresh();
		
		hostname=getHostnameTS(vtxno);
		if (hostname!=""){
			return getProvider(hostname)->lock(vtxno,flag);
		}else{
			return false;
		}
	}

	bool unlock(uint32_t vtxno)
	{
		string hostname=getHostnameTS(vtxno);
		if (hostname!="") return getProvider(hostname)->unlock(vtxno);
		return false;
	}

	uint32_t getHeadBlkno(uint32_t vtxno) { return getProvider(vtxno)->getHeadBlkno(vtxno); }
	uint32_t getTailBlkno(uint32_t vtxno) { return getProvider(vtxno)->getTailBlkno(vtxno); }

	bool read(vector<int8_t>& xblk,uint32_t vtxno,uint32_t blkno)
	{
		string hostname=getHostname(vtxno);
		Block *blk=reinterpret_cast<Block*>(xblk.data());

		if (isNative(hostname)){
			getProvider(hostname)->read(vtxno,blkno,xblk);
		}else if (isAlien(hostname)){
			if (!m_graphCache.read(vtxno,blkno,blk)){

			}
		}else{
			log_w("Oops! Never reach here!");
		}
	}

private:

	//thread safe.
	string getHostnameTS(uint32_t vtxno){
		SharedSynchronization ss(&m_subgraph2HostMapRWLock);
		uint32_t subgraphKey=SubgraphSet::VTXNO2SUBGRAPH(vtxno);
		if (m_subgraph2HostMap.find(subgraphKey)!=m_subgraph2HostMap.end()){
			return m_subgraph2HostMap[subgraphKey];
		}
		return string("");
	}
	string getHostname(uint32_t vtxno){
		return m_subgraph2HostMap[SubgraphSet::VTXNO2SUBGRAPH(vtxno)];
	}

	bool isNative(const string& hostname) { return m_self==hostname; }
	bool isAlien(const string& hostname) { return m_self!=hostname; }

	std::shared_ptr<ProviderRPC>& getProvider(const string& hostname)
	{
		return m_host2ProviderMap[hostname];
	}

	std::shared_ptr<ProviderRPC>& getProvider(uint32_t vtxno){
		return m_host2ProviderMap[m_subgraph2HostMap[SubgraphSet::VTXNO2SUBGRAPH(vtxno)]];
	}

	string     m_self;
	uint32_t 		 m_port;
	Host2PrividerMap m_host2ProviderMap;
	Subgraph2HostMap m_subgraph2HostMap;
	GraphCache       m_graphCache;
	RWLock 			 m_subgraph2HostMapRWLock;
};
}}
