#ifndef NYNN_MM_TYPES_H_BY_SATANSON
#define NYNN_MM_TYPES_H_BY_SATANSON

using namespace nynn::mm::common;
using namespace nynn::mm;

namespace nynn{namespace mm{
	
struct Vertex;
struct Edge;
template <uint32_t BLOCKSZ> union BlockType;

static uint32_t const INVALID_BLOCKNO=~0L;
static uint32_t const INVALID_VERTEXNO=~0L;

struct Vertex{
	uint32_t m_source;
	uint32_t m_data;
	uint32_t m_nedges;
	uint32_t m_blkno;
	explicit Vertex(uint32_t source):
		m_source(source),m_data(0),m_nedges(0),m_blkno(INVALID_VERTEXNO){}
};

struct Edge{
	time_t m_timestamp;
	uint32_t m_sink;
	union{
		void* 	 m_bval;
		uint64_t m_uval;
		double   m_fval;
		char 	 m_cval[8];
	}m_weight;
};

template <uint32_t BLOCKSZ> 
union BlockType
{
	char     m_data[BLOCKSZ];
	uint32_t m_indexes[BLOCKSZ/sizeof(uint32_t)];

	struct
	{
		uint32_t m_prev;
		uint32_t m_next;	
		time_t   m_infts;
		time_t   m_supts;
		uint32_t m_nedges;
	}m_header;

	static uint32_t getMaxNumberOfEdges() 
	{
		return (sizeof(BlockType)-sizeof(m_header))/sizeof(Edge); 
	}
	static uint32_t getMaxNumberOfIndexes()
	{
		return sizeof(m_indexes)/sizeof(m_indexes[0])-1;
	}

	bool isAtBottom() { return m_indexes[0]==1; }
	uint32_t isAtTop() { return m_indexes[0]==getMaxNumberOfIndexes(); }
	void initIndexBlock() { m_indexes[0]=0; }

	void pushIndex(uint32_t blkno) { m_indexes[++m_indexes[0]]=blkno; }		
	void setPrev(uint32_t blkno) { m_header.m_prev=blkno; }
	void setNext(uint32_t blkno) { m_header.m_next=blkno; }
	void setInfTimestamp(time_t t) { m_header.m_infts=t; }
	void setSupTimestamp(time_t t) { m_header.m_supts=t; }
	void setNumberOfEdges(uint32_t n) { m_header.m_nedges=n; }

	uint32_t popIndex() { return  m_indexes[m_indexes[0]--]; }		
	uint32_t getPrev() { return  m_header.m_prev; }
	uint32_t getNext() { return  m_header.m_next; }
	time_t getInfTimestamp() { return  m_header.m_infts; }
	time_t getSupTimeStamp() { return  m_header.m_supts; }
	uint32_t getNumberOfEdges() { return  m_header.m_nedges; }

	void setHeader(
			uint32_t prev,uint32_t next,time_t infts,time_t supts,uint32_t n)
	{
		setPrev(prev);
		setNext(next);
		setInfTimestamp(infts);
		setSupTimestamp(supts);
		setNumberOfEdges(n);
	}

	Edge* getFirstEdge() 
	{
		return reinterpret_cast<Edge*>(m_data+sizeof(m_header));
	}

	void  setEdge(uint32_t i,Edge *e) { *(getFirstEdge()+i)=*e; } 

	Edge* getEdge(uint32_t i) { return getFirstEdge()+i;}
};

}}
#endif
