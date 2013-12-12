namespace cpp nynn.mm

service Provider{
	i32 createSubgraph(1:string basedir);
	i32 destroySubgraph(1:string basedir);		
	i32 attachSubgraph(1:string basedir);
	i32 detachSubgraph(1:string basedir)		
	list<i32>  getSubgraphKeys();
	
	i32 lock(1:i32 vtxno,2:i32 flag);
	i32 unlock(1:i32 vtxno);
	
	i32 getSize(1:i32 vtxno);
	i32 getHeadBlkno(1:i32 vtxno);
	i32 getTailBlkno(1:i32 vtxno);
	list<byte> read(1:i32 vtxno,2:i32 blkno);
	
	i32 insertPrev(1:i32 vtxno, 2:i32 nextBlkno, 3:list<byte> blk);	
	i32 insertNext(1:i32 vtxno, 2:i32 prevBlkno, 3:list<byte> blk);	
	i32 remove(1:i32 vtxno,2:i32 blkno);

	i32 unshift(1:i32 vtxno, 2:list<byte> newHeadBlk);
	i32 shift(1:i32 vtxno);
	i32 push(1:i32 vtxno,2:list<byte> newTailBlk);
	i32 pop(1:i32 vtxno)
}
