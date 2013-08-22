#include <public.h>
#include <boss.h>
#include <sys/time.h>
class Ass1:public Assignment{
	private:
		string msg;
	public:
		Ass1(string* msg){
			this->msg=*msg;
		}
		virtual void exec(){
			cout<<msg<<endl;
		}

		virtual ~Ass1(){
		}
};
int main (){
	Boss *boss=new Boss(2,10);
	boss->assign(new Ass1(new string("Hi ranpanf")));
	boss->assign(new Ass1(new string("Hello ranpanf")));
	boss->assign(new Ass1(new string("my name is ranpanf")));
	boss->assign(new Ass1(new string("how do you do")));
	boss->assign(new Ass1(new string("nice to see you!")));
	cout<<"begin sleep"<<endl;
	struct timeval tvs,tve;
	gettimeofday(&tvs,NULL);
	cout<<boss->timedcease(1)<<endl;
	gettimeofday(&tve,NULL);
	cout<<tve.tv_sec-tvs.tv_sec<<"s"
		<<(tve.tv_usec-tvs.tv_usec)/1000<<"ms"<<endl;
	cout<<"wake up now"<<endl;
	delete boss;
}
