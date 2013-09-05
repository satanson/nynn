#include<public.h>
#include<socket.h>
#include<nynn.h>
int main(){
	char buff[64],hostname[32];
	sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = gethostaddr(hostname,32);
	saddr.sin_port = htons(30001);
	Socket so(saddr);
	so.listen(10);
	cout<<"host "<<hostname<<" is listenning..."<<endl;
	Socket so1=so.accept();
	memset(buff,0,64);

	if (so1.recv(buff,64)!=64){
		exit_on_error(errno,"failed to recv!");
	}
	
	cout<<"recv: "<<buff<<endl;
	
	memset(buff,0,64);
	cout<<"send: ";
	cin.getline(buff,64);
	if (so1.send(buff,64)!=64){
		exit_on_error(errno,"failed to send!");
	}
	
	return 0;

}
