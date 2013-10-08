#ifndef PUBLIC_H_BY_SATANSON
#define PUBLIC_H_BY_SATANSON

#include<iostream>
#include<fstream>
#include<algorithm>
#include<map>

#include<cerrno>
#include<cstdlib>
#include<cstring>
#include<ctime>
#include<csignal>
#include<cstdarg>
#include<cassert>

#include<pthread.h>

#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/shm.h>
#include<sys/epoll.h>
#include<netdb.h>
#include<unistd.h>
#include<fcntl.h>

using namespace std;


uint32_t gethostaddr(char *hostname,size_t size);
char* ltrim(const char *chars,const char *src, char *dest);
char* rtrim(const char *chars,const char *src, char *dest);
char* chop(const char ch,const char *src,char *dest);

void setNonBlocking(int sockfd);
void initSockaddr(struct sockaddr_in *addr_ptr,const char*host,short port);


void __exit_on_error(
		ostream &out,
		const char*file,
		const int line,
		const char *func,
		const int errnum,
		const char*msg,
		bool thread   
		);

typedef enum{
	INFO=0,
	WARN=1,
	ERROR=2
}logtype_t;
void __log(
		ostream &out,
		const char *file,
		const int line,
		const char *func,
		const logtype_t type,
		int errnum,
		const char *fmt,
		...
		);

enum{
	PROCESS=0,
	THREAD=1
};
#define info(msg,...) __log(cerr,__FILE__,__LINE__,__FUNCTION__,INFO,0,(msg),##__VA_ARGS__)
#define warn(msg,...) __log(cerr,__FILE__,__LINE__,__FUNCTION__,WARN,0,(msg),##__VA_ARGS__)
#define error(msg,...) __log(cerr,__FILE__,__LINE__,__FUNCTION__,ERROR,errno,(msg),##__VA_ARGS__)

#define thread_exit_on_error(errnum,msg) \
	__exit_on_error(cerr,__FILE__,__LINE__,__FUNCTION__,errnum,msg,THREAD)
#define exit_on_error(errnum,msg) \
	__exit_on_error(cerr,__FILE__,__LINE__,__FUNCTION__,errnum,msg,PROCESS)
class getlock_t{
	pthread_mutex_t *lock;
public:
	getlock_t(pthread_mutex_t*lock)
	{

		this->lock=lock;
		pthread_mutex_lock(this->lock);
	}
	~getlock_t(){
		pthread_mutex_unlock(this->lock);
	}
};

#endif
