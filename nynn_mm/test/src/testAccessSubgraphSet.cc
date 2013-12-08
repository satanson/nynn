#include<test.h>

int main(int argc,char**argv)
{
	string basedir=argv[1];
	uint32_t vtxno=strtoul(argv[2],NULL,0);
	string srcFilePath=argv[3];
	string dstFilePath=argv[4];

	SubgraphSet sgs(argv[1]);
	MmapFile srcMmapFile(srcFilePath);
	uint32_t fileLength=srcMmapFile.getLength();
	MmapFile dstMmapFile(dstFilePath,fileLength);
	char *srcBase=static_cast<char*>(srcMmapFile.getBaseAddress());
	char *dstBase=static_cast<char*>(dstMmapFile.getBaseAddress());
	Block blk;
	CharContent *content=blk;
	//copy source file to subgraphset.
	sgs.lock(0,SubgraphSet::IS_WRITABLE|SubgraphSet::IS_BLOCKING);
	uint32_t i=CharContent::CONTENT_CAPACITY;
	while(i<fileLength){
		content->resize(CharContent::CONTENT_CAPACITY);
		std::copy(srcBase+i-CharContent::CONTENT_CAPACITY,srcBase+i,content->begin());
		sgs.push(0,&blk);
		i+=CharContent::CONTENT_CAPACITY;
	}
	if (i-fileLength>0){
		content->resize(i-fileLength);
		std::copy(srcBase+i-CharContent::CONTENT_CAPACITY,
				srcBase+i-fileLength,
				content->begin());
		sgs.push(0,&blk);
	}
	sgs.unlock(0);

	sgs.lock(0,SubgraphSet::IS_WRITABLE|SubgraphSet::IS_BLOCKING);
	uint32_t blkno=sgs.getHeadBlkno(0);
	i=0;
	while(blkno!=INVALID_BLOCKNO){
		sgs.read(blkno,&blk);
		std::copy(content->begin(),content->end(),dstBase+i);
		i+=content->size();
		sgs.shift(0);
		blkno=sgs.getHeadBlkno();
	}
	sgs.unlock(0);
}
