#include<nynn_mm_common.h>
#include<nynn_mm_chunk.h>
using namespace nynn::mm;
using namespace nynn::mm::common;

int main(int argc,char*argv[]){
	chunk_t<>::init(argv[1]);
	mmap_file_t chunkmf(argv[1]);
	chunk_t<> *chk=chunk_t<>::get_chunk(chunkmf.get_base());
}
