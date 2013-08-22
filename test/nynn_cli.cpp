#include<public.h>
#include<boss.h>
#include<socket.h>
#include<sys/epoll.h>
#include<iostream>

#define MAX_EVENTS 64
#define LISTEN_BACKLOG 32
#define BUFF_SIZE 4096
using namespace std;

int main(int argc,char*argv[])
{
	int epollfd,readyfds;
	char host[INET_ADDRSTRLEN];
	char recvBuff[BUFF_SIZE],sendBuff[BUFF_SIZE];
	if ((epollfd=epoll_create(MAX_EVENTS))==-1){
		exit_on_error(errno,"failed to call epoll_create");
	}
	
	Socket connSock(-1,1);
	int connSockfd = connSock.getSockfd();


	struct epoll_event ev,readyEvents[MAX_EVENTS];

	ev.data.fd = connSockfd;
	ev.events = EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLET;	
	
	epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);

	if (connSock.connect(argv[1],atoi(argv[2]))==-1 && errno==EINPROGRESS){
		cout<<"connection operation is in-complete"<<endl;
	}

	readyfds=epoll_wait(epollfd,readyEvents,MAX_EVENTS,-1);
	if (readyfds==-1){
		exit_on_error(errno,"failed to call epoll_wait");
	}else if (readyfds==1){
		struct sockaddr addr;
		socklen_t len=sizeof(addr);
		if (getpeername(connSock.getSockfd(),&addr,&len)==-1 && errno==ENOTCONN){
			int errnum;
			socklen_t len=sizeof(errnum);
			getsockopt(connSock.getSockfd(),SOL_SOCKET,SO_ERROR,&errnum,&len);
			exit_on_error(errnum,"failed to connect");
		}
	}

	cout<<"connection established"<<endl;
	cout<<"Server:["<<connSock.getRemoteHost(host)<<":"
		<<connSock.getRemotePort()<<"]"<<endl;
	cout<<"Client:["<<connSock.getLocalHost(host)<<":"
		<<connSock.getLocalPort()<<"]"<<endl;
	while (1){
		readyfds=epoll_wait(epollfd,readyEvents,MAX_EVENTS,-1);
		if (readyfds==-1){
			exit_on_error(errno,"failed to call epoll_wait");
		}
		for (int i=0;i<readyfds;i++){
			if (readyEvents[i].events&EPOLLIN){
				Socket sock(readyEvents[i].data.fd,1);
				ssize_t rsize;
				do{
					bzero(recvBuff,sizeof(recvBuff));
					rsize=sock.recv(recvBuff,BUFF_SIZE-1);
					if (rsize==0){
						cout<<"the peer has performed a orderly shutdown"<<endl;
						sock.shutdown();
					}else if(rsize==-1 && errno!=EAGAIN){
						cout<<"a error occurred when recv"<<endl;
						sock.shutdown();
					}else {
						cout<<"recv from: ["<<sock.getRemoteHost(host)<<":"<<sock.getRemotePort()
							<<"]:"<<recvBuff<<endl;
					}
				}while(rsize==BUFF_SIZE-1);
				
			}
			if (readyEvents[i].events&EPOLLOUT){
				Socket sock(readyEvents[i].data.fd,1);
				bzero(sendBuff,sizeof(sendBuff));
				cin.getline(sendBuff,sizeof(sendBuff));

				ssize_t rsize=sock.send(sendBuff,strlen(sendBuff));

				if (rsize==-1){
					cout<<"a error occurred when send"<<endl;
					sock.shutdown();
				}else{
					cout<<"send to: ["<<sock.getRemoteHost(host)<<":"<<sock.getRemotePort()
						<<"]:"<<sendBuff<<endl;
				}
			}
			if (readyEvents[i].events&EPOLLERR){
				Socket sock(readyEvents[i].data.fd);
				sock.shutdown();
			}
		}
	}
	return 0;

}
