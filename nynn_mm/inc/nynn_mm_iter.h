#ifndef NYNN_MM_ITER_BY_SATANSON
#define NYNN_MM_ITER_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_chunk.h>
#include<nynn_mm_cache.h>
#include<nynn_mm_meta.h>
#include<nynn_mm_context.h>

using namespace nynn::mm::common;
using namespace nynn::mm;
namespace nynn{namespace mm{

class iter_t;
class vertex_iter_t;
class vertex_local_iter_t;
class vertex_all_iter_t;
class edge_iter_t;

template<typename T>
class iter_t{
protected:
	typename T _next;
public:
	iter_t();
	virtual bool has_next()=0;
	virtual T* next()=0;
	virtual ~iter_t(){}
};

class vertex_iter_t:public iter_t<vertex_t>{
public:
	vertex_iter_t();
	virtual bool has_next()=0;
	virtual vertex_t* next()=0;
	virtual ~iter_t(){}

};

class vertex_local_iter_t:public vertex_iter_t{
public:
	vertex_local_iter_t();
	bool has_next()
	{

	}

	vertex_t* next()
	{

	}

	~vertex_local_iter_t();
};

class vertex_all_iter_t:public vertex_iter_t{
public:
	vertex_all_iter_t();
	bool has_next()
	{

	}

	vertex_t* next()
	{

	}

	~vertex_all_iter_t();

};

class edge_iter_t:public iter_t<edge_t>{
public:
	edge_iter_t();
	bool has_next()
	{

	}

	edge* next()
	{

	}

	~edge_iter_t();

};

}}}
#endif
