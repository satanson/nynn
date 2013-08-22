#include<public.h>
int main()
{
	char hostname[128];
	int hostaddr=gethostaddr(hostname,128);
	char dotname[128];
	if (0!=hostaddr){
		inet_ntop(AF_INET,(void*)&hostaddr,dotname,INET_ADDRSTRLEN);
		cout<<"hostname:"<<hostname<<endl;
		cout<<"hostaddr:"<<dotname<<endl;
	}
	return 0;
}
