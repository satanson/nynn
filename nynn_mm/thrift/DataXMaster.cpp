// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "DataXServer.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::nynn::mm;

class DataXServerHandler : virtual public DataXServerIf {
 public:
  DataXServerHandler() {
    // Your initialization goes here
  }

  void submit(recommend_self_t& _return, const recommend_self_t& re) {
	  GlobalCntl.sign(re);
	  map<inetaddr_t>
	  _return._inetaddrtab.resize(0);
	  _return._inetaddrtab.insert(_return._inetaddrtab.begin(),GlobalCntl.iaddr_table.
  }

  bool update(const std::vector<std::vector<int8_t> > & data) {
    // Your implementation goes here
    printf("update\n");
  }

};
struct GlobalControl_t{
	int _nslave;
	vector<xchunk_entry_t> _chk_table;
	map<uint32_t,inetaddr_t> _iaddr_table;
	map<uint32_t,shared_ptr<DataXceiverClient> > _xceiver_table;
	unique_ptr<monitor_t> _monitor;
	map<uint32_t,shared_ptr<monitor_t> > _barrier;

	GlobalControl_t():_nslave(0),monitor(new monitor_t())
	{
		const char* cfgfile getenv("NYNN_MM_CONFIG_FILE");
		vector<inetaddr_t> iaddrs;
		loadconfig(cfgfile,iaddrs);
		if(iaddrs.size()==0){
			log_w("config file '%s' is ill-formed or can't load config file");
			exit(0);
		}
		for(vector<inetaddr_t>::iterator it=iaddrs.begin();it!=iaddrs.end();it++){
			_iaddr_table[it->_ipaddr]=*it;
			_barrier[it->_ipaddr]->reset(new monitor_t());
			pthread_mutex_lock(_barrier[it->_ipaddr]->get());
		}
	}

	void sign(recommend_self_t & re){
		uint32_t ipaddr=re._inetaddrtab[0]._ipaddr;
	
		
		do{
			//enter critical area
			synchronized_t s(_monitor->mutex());

			//_iaddr_table[ipaddr]=re._inetaddrtab[0];
			if (_iaddr_table.find(ipaddr)==_iaddr_table.end()){
				log_w("An invalid DataXceiver trys to access DataXMaster");
				exit(0);
			}

			//insert partial chunk table into global chunk table.
			_chk_table.insert(_chk_table.end(),re._parttab.begin(),re._parttab.end());
		
			//create client for every DataXceiver.
			string  hostname=_re._inetaddrtab[0]._hostname;
			uint16_t port=_re._inetaddrtab[0]._port;
			boost::shared_ptr<TTransport> socket(new TSocket(hostname,port));	
			boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
			boost::shared_ptr<TProtocol>  protocol(new TBinaryProtocol(transport));
			_xceiver_table[ipaddr].reset(new DataXceiverClient(protocol));
			//record the number of DataXceiver.	
			_nslave++;
		}while(0);

		if (_nslave==_iaddr_table.size()){
			map<uint32_t,shared_ptr<monitor_t> >::iterator it=_barrier.begin();
			for(;it!=_barrier.end();it++){
				pthread_mutex_unlock(it->second->mutex());
			}
		}
		pthread_mutex_lock(_barrier[ipaddr]->mutex());
		pthread_mutex_unlock(_barrier[ipaddr]->mutex());
	}
}GlobalCntl;


int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<DataXServerHandler> handler(new DataXServerHandler());
  shared_ptr<TProcessor> processor(new DataXServerProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

