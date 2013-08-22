#include <socket.h>
#include <fcntl.h>
static void setNonBlocking(int sockfd)
{
	int flags;
	if ((flags=fcntl(sockfd,F_GETFL))==-1){
		exit_on_error(errno,"failed to get flags of sockfd ");
	}
	flags|=O_NONBLOCK;
	if (fcntl(sockfd,F_SETFL,flags)==-1){
		exit_on_error(errno,"failed to set flags of sockfd ");
	}
}

static void initSockaddr(struct sockaddr_in *addr_ptr,const char*host,short port)
{
	bzero(addr_ptr,sizeof(*addr_ptr));
	addr_ptr->sin_family = AF_INET;
	if (inet_pton(AF_INET,host,&addr_ptr->sin_addr)!=1){
		exit_on_error(errno,"failed to convert a string to AF_INET address");
	}
	addr_ptr->sin_port = htons(port);

}

Socket::Socket(int sockfd,int nonblocking)
{
	if (sockfd<0){
		this->sockfd=socket(AF_INET,SOCK_STREAM,0);
		if(this->sockfd<0){
			exit_on_error(errno,"failed to create socket");
		}
		if (nonblocking!=BLOCKING){
			setNonBlocking(this->sockfd);
		}
	}else{
		this->sockfd=sockfd;
	}
}

//Socket::Socket(int nonblocking)
//{
//	this->sockfd=socket(AF_INET,SOCK_STREAM,0);
//	if(this->sockfd<0){
//		exit_on_error(errno,"failed to create socket");
//	}
//	if (nonblocking!=BLOCKING){setNonBlocking(this->sockfd);}
//
//}

Socket::~Socket()
{
}

Socket::Socket(const char* host,short port,int nonblocking)
{
	struct sockaddr_in addr;
	initSockaddr(&addr,host,port);

	this->sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (sockfd<0){
		exit_on_error(errno,"failed to create socket");
	}
	if(bind(this->sockfd,(struct sockaddr*)&addr,sizeof(addr))!=0){
		exit_on_error(errno,"failed to bind socket");

	}
	if(nonblocking!=BLOCKING){
		setNonBlocking(this->sockfd);
	}
	
}

Socket::Socket(sockaddr_in addr,int nonblocking)
{
	this->sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (sockfd<0){
		exit_on_error(errno,"failed to create socket");
	}
	if(bind(this->sockfd,(struct sockaddr*)&addr,sizeof(addr))!=0){
		exit_on_error(errno,"failed to bind socket");

	}
	if(nonblocking!=BLOCKING){
		setNonBlocking(this->sockfd);
	}

}
int Socket::listen(int backlog)
{
	if(::listen(this->sockfd,backlog)!=0){
		exit_on_error(errno,"failed to listen");
	}
	return this->sockfd;

}

Socket Socket::accept(int nonblocking)
{
	struct sockaddr_in addr;
	socklen_t addr_length=sizeof(addr);
	
	int conn_sockfd=::accept(this->sockfd,(struct sockaddr*)&addr,&addr_length);
	if (conn_sockfd<0){
		exit_on_error(errno,"failed to accept connection");
	}

	if(nonblocking!=BLOCKING){
		setNonBlocking(conn_sockfd);
	}
	return Socket(conn_sockfd,nonblocking);
}

int Socket::connect(const char* host,short port)
{
	
	struct sockaddr_in addr;
	initSockaddr(&addr,host,port);
	return ::connect(this->sockfd,(struct sockaddr*)&addr,sizeof(addr));
}
int Socket::connect(sockaddr_in addr)
{
	return ::connect(this->sockfd,(struct sockaddr*)&addr,sizeof(addr));
}
int Socket::send(const char *buff,int length)
{
	return write(this->sockfd,buff,length);
}

int Socket::recv(char *buff,int length)
{
	return read(this->sockfd,buff,length);
}

uint32_t Socket::getremotehost(char*ip,size_t size)
{
	struct sockaddr_in raddr;
	socklen_t raddr_len;
	getpeername(this->sockfd,(struct sockaddr*)&raddr,&raddr_len);
	if (NULL==inet_ntop(AF_INET,&raddr.sin_addr.s_addr,ip,size)){
		exit_on_error(errno,"failed to convert binary ip to string");
	}
	return raddr.sin_addr.s_addr;
}

uint32_t Socket::getlocalhost(char*ip,size_t size)
{
	struct sockaddr_in laddr;
	socklen_t laddr_len;
	getsockname(this->sockfd,(struct sockaddr*)&laddr,&laddr_len);
	if (NULL==inet_ntop(AF_INET,&laddr.sin_addr.s_addr,ip,size)){
		exit_on_error(errno,"failed to convert binary ip to string");
	}
	return laddr.sin_addr.s_addr;
}

unsigned short Socket::getremoteport()
{
	struct sockaddr_in raddr;
	socklen_t raddr_len;
	getpeername(this->sockfd,(struct sockaddr*)&raddr,&raddr_len);
	return (unsigned short)ntohs(raddr.sin_port);
}

unsigned short Socket::getlocalport()
{
	struct sockaddr_in laddr;
	socklen_t laddr_len;
	getsockname(this->sockfd,(struct sockaddr*)&laddr,&laddr_len);
	return (unsigned short)ntohs(laddr.sin_port);
}

int Socket::shutdown()
{
	return ::shutdown(this->sockfd,SHUT_RDWR);
}

int Socket::getsockfd()const
{
	return this->sockfd;
}

