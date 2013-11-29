#ifndef TEST_H_BY_SATANSON
#define TEST_H_BY_SATANSON
#include<nynn_mm_types.h>
#include<nynn_mm_subgraph_storage.h>
using namespace nynn::mm;
typedef nynn::mm::BlockType<1<<9> Block;
typedef nynn::mm::SubgraphStorage<9,12,1<<16,~0L,1<<16,1<<10,64> Subgraph;
#endif
