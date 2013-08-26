#include<nynn.h>
int main()
{
	char *rbuff;
	size_t rbuffsize;
	nynn_tap_t tap;
	while(true){
		if(tap.read(&rbuff,&rbuffsize)!=0){
			cout<<"failed to read!"<<endl;
			continue;
		}
		cout<<rbuff<<endl;
		delete rbuff;
	}
	return 0;
}
