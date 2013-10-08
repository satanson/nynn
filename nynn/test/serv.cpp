#include<public.h>
#include<socket.h>
#include<nynn.h>
int main(){
	char buff[INET_ADDRSTRLEN],hostname[32];
	sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = gethostaddr(hostname,32);
	saddr.sin_port = htons(30001);
	Socket so(saddr);
	so.listen(10);
	cout<<"host "<<hostname<<" is listenning..."<<endl;
	while(true){
		Socket so1=so.accept();
		int pid=fork();
		if (pid>0){
			so1.close();
		}else if (pid==0){
			//cout<<"local host:"<<so1.getlocalhost(buff)<<endl;
			//cout<<"local port:"<<so1.getlocalport()<<endl;
			//cout<<"remote host:"<<so1.getremotehost(buff,INET_ADDRSTRLEN)<<endl;
			//cout<<"remote port:"<<so1.getremoteport()<<endl;

			int shmid;
			so1.recv((char*)&shmid,4);
			//cout<<"shmid="<<shmid<<endl;
			char *shm;
			//sleep(1);
			nynn_shmat(shmid,(void**)&shm,4096,true);
			cout<<shm<<endl;
			nynn_shmdt(shm);
			return 0;
		}else{
			cout<<"failed to fork a process"<<endl;
		}
	}
	return 0;

}
