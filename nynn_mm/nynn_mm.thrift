namespace * nynn.mm

struct xchunk_entry_t{
	1:required string _path;
	2:required byte _flag;
	3:required i64 _minvtx;
	4:required i64 _maxvtx;
	5:required i32 _where
}

struct xedge_t{
	1:required i64 _timestamp;
	2:required i64 _sink;
	3:required i64 _value;
}

struct xvertex_t{
	1:required i64 _source;
	2:required list<xedge_t> _edge
}

struct xinetaddr_t{
	1:required string _hostname;
	2:required i32 _hostaddr;
	3:required i16 _port
}

struct xletter_t{
	1:required list<xinetaddr_t> _iaddr_table;
	2:required list<xchunk_entry_t> _chk_table;
}


service DataXMaster{
	xletter_t submit(1:xletter_t quest);
	bool update(1:list<xedge_t> edge)
}
service DataXPeer{
	//app fetch data from DataXceiver.
	list<list<byte> > read(1:string hostname,2:i64 vtxno,3:i32 blkno);
	list<list<byte> > fetch(1:i64 vtxno,2:i32 blkno);
	//DataXServer post data to DataXceiver.
	bool write(1:xvertex_t vtx)
}
