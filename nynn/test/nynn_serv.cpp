#include<public.h>
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
	
	Socket listenSock(argv[1],atoi(argv[2]),1);
	struct epoll_event ev,ev1,readyEvents[MAX_EVENTS];

	ev.data.fd = listenSock.getsockfd();
	ev.events = EPOLLIN|EPOLLET;	
	epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);
	
	listenSock.listen(LISTEN_BACKLOG);

	while (1){
		readyfds=epoll_wait(epollfd,readyEvents,MAX_EVENTS,-1);
		if (readyfds==-1){
			exit_on_error(errno,"failed to call epoll_wait");
		}
		for (int i=0;i<readyfds;i++){
			if (readyEvents[i].data.fd == listenSock.getsockfd()){
				Socket connSock=listenSock.accept(1);
				cout<<"host: "<<connSock.getremotehost(host)<<
					" port: "<<connSock.getremoteport()<<endl;

				ev.data.fd = connSock.getsockfd();
				ev.events = EPOLLOUT|EPOLLET;
				
				ev1.data.fd = dup(ev.data.fd);
				ev1.events  = EPOLLIN|EPOLLET;

				epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);
				epoll_ctl(epollfd,EPOLL_CTL_ADD,ev1.data.fd,&ev1);

			}else {
				if (readyEvents[i].events&EPOLLIN){
					cout<<"EPOLLIN"<<endl;
					
					Socket sock(readyEvents[i].data.fd,1);
					ssize_t rsize;
					do{
						bzero(recvBuff,sizeof(recvBuff));
						rsize=sock.recv(recvBuff,BUFF_SIZE-1);
						if (rsize==0){
							cout<<"the peer has performed a orderly shutdown"<<endl;
							sock.shutdown();
							break;
						}else if(rsize==-1 && errno!=EAGAIN){
							cout<<"a error occurred when recv"<<endl;
							sock.shutdown();
							break;
						}else {
							cout<<"recv from: ["<<sock.getremotehost(host)<<":"<<sock.getremoteport()
								<<"]:"<<recvBuff<<endl;
						}
					}while(rsize==BUFF_SIZE-1);
					
					
				}
				if (readyEvents[i].events&EPOLLOUT){
					cout<<"EPOLLOUT"<<endl;
					//for(int j=0;j<1000000;j++)j*j;
					//epoll_ctl(epollfd,EPOLL_CTL_DEL,readyEvents[i].data.fd,NULL);
					//ev.data.fd=readyEvents[i].data.fd;
					//ev.events = EPOLLOUT|EPOLLERR|EPOLLET;
					//epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);

					Socket sock(readyEvents[i].data.fd,1);
					bzero(sendBuff,sizeof(sendBuff));
					cin.getline(sendBuff,sizeof(sendBuff));

					ssize_t rsize=sock.send(sendBuff,strlen(sendBuff));

					if (rsize==-1){
						cout<<"a error occurred when send"<<endl;
						sock.shutdown();
					}else{
						cout<<"send to: ["<<sock.getremotehost(host)<<":"<<sock.getremoteport()
							<<"]:"<<sendBuff<<endl;
					}
				}
				if (readyEvents[i].events&EPOLLERR){
					cout<<"EPOLLERR"<<endl;
					Socket sock(readyEvents[i].data.fd);
					sock.shutdown();
				}
			}	
		}
	}
	return 0;
}
