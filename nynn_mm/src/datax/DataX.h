#ifndef DATAX_H_BY_SATANSON
#define DATAX_H_BY_SATANSON

#include<arpa/inet.h>

#include<nynn_mm.h>

#include<DataXMaster.h>
#include<DataXPeer.h>
#include<nynn_mm_constants.h>
#include<nynn_mm_types.h>


#include<thrift/server/TNonblockingServer.h>
#include<thrift/protocol/TBinaryProtocol.h>
#include<thrift/transport/TBufferTransports.h>
#include<thrift/transport/TSocket.h>
#include<thrift/concurrency/Thread.h>
#include<thrift/concurrency/PosixThreadFactory.h>

namespace nynn{namespace mm{
vector<xinetaddr_t>& loadconfig(const char*cfgpath,vector<xinetaddr_t> &cfg);
}}
#endif
