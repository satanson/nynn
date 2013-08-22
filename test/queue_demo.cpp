#include<concurrent_queue.h>
#include<ctime>
int main(int argc,char*argv[])
{
	concurrent_queue<int> q;
	for(int i=0;i<atoi(argv[1]);i++){
		q.push(i);
	}
	while(!q.empty()){
		cout<<q.pop()<<",";
	}
	cout<<endl;
	cout<<"sizeof(queue)="<<sizeof(q)<<endl;
	cout<<"sizeof(pthread_cond_t)"<<sizeof(pthread_cond_t)<<endl;
	cout<<"sizeof(pthread_mutex_t)"<<sizeof(pthread_mutex_t)<<endl;
	return 0;
}
