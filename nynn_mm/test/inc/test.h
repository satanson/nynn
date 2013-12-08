#ifndef TEST_H_BY_SATANSON
#define TEST_H_BY_SATANSON
#include<nynn_mm_types.h>
#include<nynn_mm_subgraph_storage.h>
#include<nynn_mm_subgraph_cache.h>
#include<nynn_mm_subgraph_set.h>
using namespace nynn::mm;
typedef nynn::mm::SubgraphStorageType<9,12,1<<16,~0L,16,1<<10,64> Subgraph;
typedef nynn::mm::SubgraphCacheType<9,1024*64,1024,64> SubgraphCache;
typedef nynn::mm::Subgraph::Block Block;
typedef Block::Tcontent<char> CharContent;
typedef nynn::mm::SubgraphSetType<Subgraph,Block,CharContent> SubgraphSet;
#endif
