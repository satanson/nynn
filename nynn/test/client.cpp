#include <socket.h>
#include <public.h>
#include <iostream>

using namespace std;

int main(int argc,char* argv[])
{
	Socket sock;
	char *host=argv[1];
	uint16_t port=(uint16_t)atoi(argv[2]);
	cout<<"connect "<<host<<":"<<port<<endl;
	sock.connect(host,port);
	char buff[64];
	sock.getremotehost(buff);
	cout<<"Server:"<<buff<<":"<<sock.getremoteport()<<endl;
	sock.getlocalhost(buff);
	cout<<"Client:"<<buff<<":"<<sock.getlocalport()<<endl;
	
	bzero(buff,sizeof(buff));
	cout<<"send: ";
	cin.getline(buff,64);
	if (sock.send(buff,64)!=64){
		exit_on_error(errno,"failed to send!");
	}
	
	bzero(buff,sizeof(buff));
	if (sock.recv(buff,64)!=64){
		exit_on_error(errno,"failed to recv!");
	}
	cout<<"recv: "<<buff<<endl;
	
	return 0;
}

