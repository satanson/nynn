#include<nynn_mm_common.h>
using namespace nynn::mm::common;
void foo1()throw (nynn_error_t);
void foo2()throw (nynn_error_t);
void foo3()throw (nynn_error_t);

void foo1()throw (nynn_error_t){
	foo2();
}

void foo2()throw (nynn_error_t){
	foo3();
}

void foo3()throw (nynn_error_t){
	throw nynn_error_t(3,__FILE__,__LINE__,__FUNCTION__);
}
int main()
{
	try{
		foo3();
	}catch(nynn_error_t& err){
		err.print_backtrace();
	}
	return 0;

}
