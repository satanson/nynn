#include<public.h>
#include<socket.h>
#include<nynn.h>
int main(int argc,char*argv[]){
	Socket so;
	char buff[INET_ADDRSTRLEN];
	char *host=argv[1];
	uint16_t port=(uint16_t)atoi(argv[2]);
	cout<<"connect "<<host<<":"<<port<<endl;
	so.connect(host,port);
	//cout<<"local host:"<<so.getlocalhost(buff)<<endl;
	//cout<<"local port:"<<so.getlocalport()<<endl;
	//cout<<"remote host:"<<so.getremotehost(buff)<<endl;
	//cout<<"remote port:"<<so.getremoteport()<<endl;
	char *shm;
	int shmid=nynn_shmat(-1,(void**)&shm,4096,false);
	//cout<<"shmid="<<shmid<<endl;
	cin.getline(shm,4096);

	so.send((char*)&shmid,4);
	nynn_shmdt(shm);
	return 0;
}
