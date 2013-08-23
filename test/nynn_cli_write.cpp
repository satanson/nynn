#include<nynn.h>
int main(int argc,char*argv[])
{
	char wbuff[4092];
	char *rbuff;
	size_t rbuffsize;
	uint32_t inetaddr[2];
	inet_pton(AF_INET,argv[1],&inetaddr[0]);
	inet_pton(AF_INET,argv[2],&inetaddr[1]);
	while(true){
		memset(wbuff,0,4092);
		if(cin.eof())break;
		cin.getline(wbuff,4092);
		nynn_write(inetaddr,2,wbuff,4092);
	}
	return 0;
}
