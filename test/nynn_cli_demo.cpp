#include<nynn.h>
int main(int argc,char*argv[])
{
	size_t inetaddrsize=atoi(argv[1]);
	uint32_t *inetaddr=new uint32_t[inetaddrsize];
	size_t i=0;
	for (i=0;i<inetaddrsize;i++){
		inet_pton(AF_INET,argv[2+i],&inetaddr[i]);
	}
	
	nynn_tap_t tap;

	size_t wbuffsize=atoi(argv[2+i]);
	char *wbuff=new char[wbuffsize];
	char *rbuff;
	size_t rbuffsize;
	while(true){
		memset(wbuff,0,wbuffsize);
		if(cin.eof())break;
		cin.getline(wbuff,wbuffsize);
		tap.write(inetaddr,inetaddrsize,wbuff,wbuffsize);
		if(tap.read(&rbuff,&rbuffsize)!=0){
			cout<<"failed to read!"<<endl;
			continue;
		}
		cout<<rbuff<<endl;
		delete rbuff;
	}
	return 0;
}
