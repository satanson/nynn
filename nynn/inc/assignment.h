#ifndef ASSIGNMENT_H_BY_SATANSON
#define ASSIGNMENT_H_BY_SATANSON
#include<public.h>
#include<socket.h>
#include<boss.h>
class Assignment;
class DistMemManager
{

};

class Assignment
{
	protected:
	public:
		Assignment();
		virtual ~Assignment();
		virtual void exec()=0;
};

class AcceptAss:public Assignment
{
	private:
		int listenSock;
		int epollfd;
	public:
		AcceptAss(Socket listenSock, int epollfd);
		~AcceptAss();
	
}
class ConnectAss:public Assignment
{
	private:
		Socket sock;
	public:
		ConnectAss(Socket sock);
		~ConnectAss();
	
}
class RecvAss:public Assignment
{
	private:
		Socket sock;
	public:
		RecvAss(Socket sock);
		~RecvAss();
	
}
class SendAss:public Assignment
{
	private:
		Socket sock;
	public:
		SendAss(Socket sock);
		~SendAss();
	
}
class ExceptAss:public Assignment
{
	private:
		Socket sock;
	public:
		ExceptAss(Socket sock);
		~ExcepttAss();
	
}

#endif
