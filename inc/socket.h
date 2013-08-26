/*******************************
 * socket's OOP wrapper
 * ****************************/
#ifndef SOCKET_H_BY_SATANSON
#define SOCKET_H_BY_SATANSON
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <public.h>
enum{
	BLOCKING=0,
	NONBLOCKING=1
};
class Socket{
	private:
		int sockfd;
	public:
		Socket(const char*host,short port,int nonblocking=BLOCKING);
		Socket(int fd=-1,int nonblocking=BLOCKING);
		Socket(sockaddr_in saddr,int nonblocking=BLOCKING);
		~Socket();
		int getsockfd()const;
		
		int listen(int backlog);
		Socket accept(int nonblocking=BLOCKING);
		int connect(const char* ip,short port);
		int connect(sockaddr_in addr);
		int send(const char *buff,int length,int flag=0);
		int recv(char *buff,int length,int flag=0);
		int close();
		int shutdown();
		
		int getsockname(struct sockaddr_in *saddr,socklen_t *len);
		int getpeername(struct sockaddr_in *saddr,socklen_t *len);
		uint32_t getremotehost(char*host,size_t size=INET_ADDRSTRLEN);
		uint32_t getlocalhost(char*host,size_t size=INET_ADDRSTRLEN);
		unsigned short getremoteport();
		unsigned short getlocalport();

};
#endif
