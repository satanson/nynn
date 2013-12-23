// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Agent.h"
#include<nynn_mm_config.h>
#include<nynn_mm_graph.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/Thread.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/server/TNonblockingServer.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;


using boost::shared_ptr;

using namespace  ::nynn::mm;

class AgentHandler : virtual public AgentIf {
 public:
  AgentHandler() {
    // Your initialization goes here
  }

  bool lock(const int32_t vtxno) {
    // Your implementation goes here
    printf("lock\n");
  }

  bool unlock(const int32_t vtxno) {
    // Your implementation goes here
    printf("unlock\n");
  }

  int32_t getHeadBlkno(const int32_t vtxno) {
    // Your implementation goes here
    printf("getHeadBlkno\n");
  }

  int32_t getTailBlkno(const int32_t vtxno) {
    // Your implementation goes here
    printf("getTailBlkno\n");
  }

  void read(std::vector<int8_t> & _return, const int32_t vtxno, const int32_t blkno) {
    // Your implementation goes here
    printf("read\n");
  }

};

int main(int argc, char **argv) {
	int port =strtoul(getenv("AGENT_PORT"),NULL,0);

	boost::shared_ptr<AgentHandler> handler(new AgentHandler());
	boost::shared_ptr<TProcessor> processor(new AgentProcessor(handler));
	//boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<ThreadFactory> pthreadFactory(new PosixThreadFactory());

	boost::shared_ptr<ThreadManager> threadManager=ThreadManager::newSimpleThreadManager(10,4);
	threadManager->threadFactory(pthreadFactory);

	TNonblockingServer server(processor,transportFactory,transportFactory,protocolFactory,protocolFactory,port,threadManager);
	threadManager->start();
	server.serve();
	return 0;
}

