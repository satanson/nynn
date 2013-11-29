#ifndef NYNN_MM_TYPE_H_BY_SATANSON
#define NYNN_MM_TYPE_H_BY_SATANSON

namespace nynn{namespace mm{

struct ChkTblEntry;
struct ChkTbl;
struct Chunk;


enum{
	HOST_MAX=128,
	PATH_MAX=128,
	CHUNK_TABLE_SIZE=128
};

struct ChkTblEntry{
	char host[HOST_MAX];
	char path[PATH_MAX];
	char flag;
	uint64_t minvtx;
	uint64_t maxvtx;
//
	bool operator< (ChkTblEntry const& rhs)const{
		return this->maxvtx<rhs.minvtx;
	}
};

struct ChkTbl{

};

}}
#endif

