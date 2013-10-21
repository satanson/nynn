#include<nynn_mm_common.h>
using namespace nynn::mm::common;
struct A{
	A()
	{
		string flckpath="../test/testflock.cc";
		cout<<"construct object from A"<<endl;

		unique_ptr<flock_t> flckptr(new fwlock_t(flckpath));
		raii_flock_t require(flckptr.get());

		cout<<"required file lock"<<endl;
		string oncepath="./once.tag";
		if (file_exist(oncepath)){
			mmap_file_t mf(oncepath);
			int *refcount=static_cast<int*>(mf.get_base());
			(*refcount)++;
			cout<<"now there are "<<*refcount<<" users hold resources"<<endl;
			cout<<"obtain shared resources!"<<endl;
		}else{
			mmap_file_t mf(oncepath,4);
			int *refcount=static_cast<int*>(mf.get_base());
			*refcount=1;
			cout<<"initialized shared resources!"<<endl;
		}

	}

	~A()
	{
		string flckpath="../test/testflock.cc";
		
		unique_ptr<flock_t> flckptr(new fwlock_t(flckpath));
		raii_flock_t require(flckptr.get());

		string oncepath="./once.tag";
		mmap_file_t mf(oncepath);
		int *refcount=static_cast<int*>(mf.get_base());
		(*refcount)--;
		cout<<"now there are "<<*refcount<<" users hold resources"<<endl;

		if (*refcount==0) {
			cout<<"release resources"<<endl;
			if (remove(oncepath.c_str())!=0)THROW_ERROR;
		}
		cout<<"destruct object of A"<<endl;

	}
} a;
int main(){
	cout<<"main run..."<<endl;
	int s=rand_int()%10+1;
	cout<<"main: sleep for "<<s<<" s"<<endl;
	sleep(s);
	cout<<"main: wake up"<<endl;
}
