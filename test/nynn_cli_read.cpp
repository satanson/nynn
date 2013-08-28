#include<nynn.h>
int main(int argc,char*argv[])
{
	char *rbuff;
	size_t rbuffsize;
	uint16_t port=atoi(argv[1]);
	int hose=atoi(argv[2]);
	nynn_tap_t tap(port,hose);
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
