#include<nynn_mm_common.h>
#include<memory>
using namespace std;
using namespace nynn::mm::common;
int main()
{
	mmap_file_t *mf=NULL;
	try{
		mf=new mmap_file_t("test.dat",4096);
	}catch(nynn_error_t& err){
		err.print_backtrace();
		exit(0);
	}
	void* base=mf->get_base();
	int *p=static_cast<int*>(base);
	for (int i=0;i<1024;i++){
		*p++=i;
	}
	try{
		mf->sync(MS_SYNC|MS_INVALIDATE);
	}catch(nynn_error_t& err) {
		err.print_backtrace();
	}
	log_i("ranpanf say:\"%s\"","hello satanson!");
	log_w("ranpanf say:\"%s\"","hello satanson!");
	log_e(3);
	return 0;
}
