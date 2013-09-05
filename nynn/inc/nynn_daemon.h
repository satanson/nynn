#ifndef NYNN_DAEMON_H_BY_SATANSON
#define NYNN_DAEMON_H_BY_SATANSON
#include<public.h>
#include<socket.h>
#include<nynn.h>
#include<concurrent_queue.h>
struct task_t;
struct link_t;
struct hose_t;
struct shm_manager_t;
typedef concurrent_queue<nynn_token_t*> token_queue_t;
typedef concurrent_queue<task_t*> task_queue_t;
struct task_t{
	int tasktype;
	int sockfd;
	task_t(int tasktype,int sockfd){
		this->tasktype=tasktype;
		this->sockfd=sockfd;
	}
};

struct link_t{
	char hostname[32];
	uint32_t hostaddr;
	uint16_t port;
	int rfd;
	int wfd;
	nynn_token_t *cachedfragment;
	token_queue_t wqueue;
	pthread_mutex_t wlock;
	pthread_mutex_t rlock;
	friend bool operator<(const link_t& lhs, const link_t &rhs);
};



struct hose_t{
	private:
		pthread_mutex_t lock;
		typedef map<string,token_queue_t*> hosemap_t;
		typedef hosemap_t::iterator hosemap_iterator_t;
		typedef pair<string,token_queue_t*> hosemap_pair_t;
		hosemap_t hosemap;
	public:
		hose_t()
		{
			pthread_mutex_init(&lock,NULL);
		}

		~hose_t()
		{
			pthread_mutex_destroy(&lock);
			hosemap_iterator_t iter;
			for(iter=hosemap.begin();iter!=hosemap.end();iter++){
				delete iter->second;
			}
		}

		token_queue_t* add(const char*k)
		{
			hosemap_iterator_t iter;
			token_queue_t*q;
			string key(k);	
			getlock_t get(&this->lock);	
			iter=hosemap.find(key);
			if (iter==hosemap.end()){
			   	q=new token_queue_t;
				hosemap.insert(hosemap_pair_t(key,q));
			}else{
			   	q=iter->second;
			}
			return q;
		}

		void remove(const char *k)
		{
			hosemap_iterator_t iter;
			string key(k);
			
			getlock_t get(&this->lock);	
			
			iter=hosemap.find(key);
			if(iter!=hosemap.end()){
				hosemap.erase(key);
			}
		}
};


int loadconfig(const char*cfgpath,link_t *links,size_t size);

enum{
	LINK_MAX=20,
	HOSTNAME_SIZE=32
};

extern link_t *links;
extern hose_t hoses;
extern size_t links_max;
extern size_t hose_max;
extern size_t writer_num;
extern size_t reader_num;
extern char cfgpath[];

extern size_t linksize;
extern char hostname[];
extern uint32_t hostaddr;
extern token_queue_t rqueue;

extern task_queue_t wtask_queue;
extern task_queue_t rtask_queue;

void* connector(void*args);
void* acceptor(void*args);
void* poller(void*args);
void* writer(void*args);
void* reader(void*args);
void* wresponder(void*args);
void* rresponder(void*args);
class shm_manager_t{
	private:
		size_t shmseg;
		size_t count;
		pthread_mutex_t lock;
		pthread_cond_t notempty;
		pthread_cond_t notfull;
	public:
		shm_manager_t(size_t shmseg){
			this->shmseg=shmseg;
			this->count=0;
			pthread_mutex_init(&this->lock,NULL);
			pthread_cond_init(&this->notfull,NULL);
			pthread_cond_init(&this->notempty,NULL);
		}

		int require(int shmid,void**shmaddr,size_t size,bool removal){
			int retval;
			getlock_t get(&this->lock);
			while(this->count==this->shmseg)
				pthread_cond_wait(&this->notfull,&this->lock);
			retval=nynn_shmat(shmid,shmaddr,size,removal);
			this->count++;
			pthread_cond_broadcast(&this->notempty);
			return retval;
		}
		int release(const void*shmaddr){
			int retval=0;
			getlock_t get(&this->lock);
			while(this->count==0)
				pthread_cond_wait(&this->notempty,&this->lock);
			retval=nynn_shmdt(shmaddr);
			this->count--;
			pthread_cond_broadcast(&this->notfull);
		}

};
#endif

