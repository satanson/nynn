#include "Provider.h"
#include<iostream>
#include<thrift/protocol/TBinaryProtocol.h>
#include<thrift/transport/TSocket.h>
#include<thrift/transport/TTransportUtils.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace boost;
using namespace nynn::mm;

namespace nynn{namespace mm {namespace rpc{
class ProviderRPC{
public:
	ProviderRPC(const string &host, int port)	
	{
		boost::shared_ptr<TTransport> socket(new TSocket(host,port));	
		boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
		boost::shared_ptr<TProtocol>  protocol(new TBinaryProtocol(transport));
		client.reset(new ProviderClient(protocol));
		try{
			socket->open();
		}catch(TException &tx){
		}
	}
	bool createSubgraph(const int32_t subgraphKey) { return client->createSubgraph(subgraphKey); }

	bool destroySubgraph(const int32_t subgraphKey) { return client->destroySubgraph(subgraphKey); }

	bool attachSubgraph(const int32_t subgraphKey) { return client->attachSubgraph(subgraphKey); }

	bool detachSubgraph(const int32_t subgraphKey) { return client->detachSubgraph(subgraphKey); }

	void getSubgraphKeys(std::vector<int32_t> & keys) 
	{ 
		vector<int32_t> tempKeys;
		client->getSubgraphKeys(tempKeys);
		keys.resize(tempKeys.size());
		std::copy(tempKeys.begin(),tempKeys.end(),keys.begin());
	}

	bool lock(const int32_t vtxno, const int32_t flag) { return client->lock(vtxno, flag);}

	bool unlock(const int32_t vtxno) { return client->unlock(vtxno);}

	int32_t getSize(const int32_t vtxno) { return client->getSize(vtxno);}

	int32_t getHeadBlkno(const int32_t vtxno) { return client->getHeadBlkno(vtxno);}

	int32_t getTailBlkno(const int32_t vtxno) { return client->getTailBlkno(vtxno);}

	void readAllBlknos(uint32_t vtxno,std::vector<int32_t>& blknos)
	{
		client->readAllBlknos(blknos,vtxno);
	}

	void read(const int32_t vtxno,const int32_t blkno, std::vector<int8_t> & blk) 
	{ 
		client->read(blk, vtxno, blkno);
	}

	void readn(const int32_t vtxno,const int32_t blkno, const int32_t n,std::vector<int8_t>& xblks)
	{
		client->readn(xblks,vtxno,blkno,n);
	}

	int32_t insertPrev(const int32_t vtxno, const int32_t nextBlkno, const std::vector<int8_t> & blk) 
	{ 
		return client->insertPrev(vtxno, nextBlkno, blk);
	}

	int32_t insertNext(const int32_t vtxno, const int32_t prevBlkno, const std::vector<int8_t> & blk) 
	{ 
		return client->insertNext(vtxno, prevBlkno, blk);
	}

	bool remove(const int32_t vtxno, const int32_t blkno) { return client->remove(vtxno, blkno);}

	int32_t unshift(const int32_t vtxno, const std::vector<int8_t> & newHeadBlk) 
	{ 
		return client->unshift(vtxno, newHeadBlk);
	}

	bool shift(const int32_t vtxno) { return client->shift(vtxno);}

	int32_t push(const int32_t vtxno, const std::vector<int8_t> & newTailBlk) {
	   	return client->push(vtxno, newTailBlk);
	}
	bool pop(const int32_t vtxno) { return client->pop(vtxno);}
	
private:
	std::shared_ptr<ProviderClient> client;
};
}}}

