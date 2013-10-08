#include<public.h>
int main()
{
	link_t links[20];
	loadconfig("network.cfg",links,20);
	for(size_t i=0;i<20&&strlen(links[i].hostname)!=0;i++){
		cout<<links[i].hostname<<":"
			<<links[i].hostaddr<<":"
			<<links[i].port<<endl;
	}
	return 0;
}
