/*----------------------class Assignment-------------*/
Assignment::~Assignment(){}
Assignment::Assignment(){}

/*----------------------class AcceptAss---------------*/
AcceptAss::AcceptAss(Socket listenSock,int epollfd)
{
	this->listenSock = listenSock;
	this->epollfd = epollfd;
}

AcceptAss::~AcceptAss()
{
}

void AcceptAss::exec()
{
	struct epoll_event ev;
	Socket connSock = listenSock.accept(1);
	ev.data.fd = conSock.getSockfd();
	ev.events = EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLET;
	if (epoll_ctl(this->epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev)==-1){
		pthread_exit_on_error(errno,"failed to add epoll event");
	}
}

/*-------------------------class RecvAss--------------*/
SendAss::SendAss(Socket sock)
{
	this->sock=sock;
}

void SendAss::exec()
{
	char sbuff[4096+1];
	bzero(sbuff,sizeof(sbuff));

}
