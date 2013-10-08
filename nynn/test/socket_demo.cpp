#include <socket.h>
#include <public.h>
#include <iostream>

using namespace std;

int main()
{
	Socket listenSock("192.168.1.2",0xf00f);
	listenSock.listen(20);
	Socket connSock=listenSock.accept();
	char buff[64];
	cout<<"Server: IP="<<connSock.getLocalHost(buff)<<
		" port="<<connSock.getLocalPort()<<endl;
	cout<<"Client: IP="<<connSock.getRemoteHost(buff)<<
		" port="<<connSock.getRemotePort()<<endl;
	bzero(buff,sizeof(buff));

	cout<<connSock.recv(buff,64)<<endl;
	cout<<"recv: "<<buff<<endl;
	
	cout<<"send: OK"<<endl;
	bzero(buff,sizeof(buff));
	strcpy(buff,"OK");
	connSock.send(buff,strlen(buff));
	return 0;
}
