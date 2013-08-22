#include <socket.h>
#include <public.h>
#include <iostream>

using namespace std;

int main()
{
	Socket sock;
	sock.connect("192.168.1.2",0xf00f);
	char buff[64];
	cout<<"Server: IP="<<sock.getRemoteHost(buff)<<
		" port="<<sock.getRemotePort()<<endl;
	cout<<"Client: IP="<<sock.getLocalHost(buff)<<
		" port="<<sock.getLocalPort()<<endl;
	
	bzero(buff,sizeof(buff));
	strcpy(buff,"hello world!");
	sock.send(buff,strlen(buff));

	bzero(buff,sizeof(buff));
	cout<<sock.recv(buff,64)<<endl;
	cout<<"recv: "<<buff<<endl;
	
	return 0;
}

