#ifndef NYNN_DAEMON_H_BY_SATANSON
#define NYNN_DAEMON_H_BY_SATANSON
#include<public.h>
#include<socket.h>
#include<nynn.h>
#include<concurrent_queue.h>

struct link_t{
	char hostname[32];
	uint32_t hostaddr;
	uint16_t port;
	int rfd;
	int wfd;
	nynn_token_t *cachedfragment;
	concurrent_queue<nynn_token_t*> wqueue;
	pthread_mutex_t wlock;
	pthread_mutex_t rlock;
	friend bool operator<(const link_t& lhs, const link_t &rhs);
};

struct task_t{
	int tasktype;
	int sockfd;
	task_t(int tasktype,int sockfd){
		this->tasktype=tasktype;
		this->sockfd=sockfd;
	}
};

int loadconfig(const char*cfgpath,link_t *links,size_t size);

enum{
	LINK_MAX=20,
	HOSTNAME_SIZE=32
};

extern link_t *links;
extern size_t links_max;
extern size_t writer_num;
extern size_t reader_num;
extern char cfgpath[];

extern size_t linksize;
extern char hostname[];
extern uint32_t hostaddr;
extern concurrent_queue<nynn_token_t*> rqueue;

extern concurrent_queue<task_t*> wtask_queue;
extern concurrent_queue<task_t*> rtask_queue;

void* connector(void*args);
void* acceptor(void*args);
void* poller(void*args);
void* writer(void*args);
void* reader(void*args);
void* wresponder(void*args);
void* rresponder(void*args);
#endif

