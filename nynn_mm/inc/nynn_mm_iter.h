#ifndef NYNN_MM_ITER_BY_SATANSON
#define NYNN_MM_ITER_BY_SATANSON
#include<nynn_mm_common.h>
#include<nynn_mm_local.h>
#include<nynn_mm_cache.h>


using namespace nynn::mm::common;
using namespace nynn::mm::local;
using namespace nynn::mm::cache;
namespace nynn{namespace mm{namespace iter{

class iter_t;
class vertex_iter_t;
class vertex_local_iter;
class vertex_all_iter;
class edge_iter;

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

class vertex_local_iter:public vertex_iter_t{
public:
	vertex_local_iter();
	bool has_next()
	{

	}

	vertex_t* next()
	{

	}

	~vertex_local_iter();
};

class vertex_all_iter:public vertex_iter_t{
public:
	vertex_all_iter();
	bool has_next()
	{

	}

	vertex_t* next()
	{

	}

	~vertex_all_iter();

};

class edge_iter:public iter_t<edge_t>{
public:
	edge_iter();
	bool has_next()
	{

	}

	edge* next()
	{

	}

	~edge_iter();

};

}}}
#endif
