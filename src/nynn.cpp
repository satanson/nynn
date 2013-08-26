#include<public.h>
#include<nynn.h>
#include<socket.h>
int nynn_shmat(int shmid, void **shmaddr,size_t size,bool removal)
{
	if(shmid==-1){
		shmid=shmget(IPC_PRIVATE,size,IPC_CREAT|IPC_EXCL);
		if (shmid==-1){
			error("failed to shmget");
			return -1;
		}
		*shmaddr=shmat(shmid,NULL,0);
		if (*shmaddr==(void*)-1){
			error("failed to shmat");
			return -1;
		}
	}else{
		*shmaddr=shmat(shmid,NULL,0);
		if (*shmaddr==(void*)-1){
			error("failed to shmat");
			return -1;
		}
	}
	if (removal){
		if (shmctl(shmid,IPC_RMID,NULL)==-1){
			warn("failed to remove shared memory");
		}
	}
	return shmid;
}
int nynn_shmdt(const void*shmaddr)
{
	if (shmdt(shmaddr)==-1){
		error("failed to shmdt");
		return -1;
	}
	return 0;
}

nynn_tap_t::nynn_tap_t()
{
	pthread_mutex_init(&this->wlock,NULL);
	pthread_mutex_init(&this->rlock,NULL);
	Socket wso;
	if (wso.connect("127.0.0.1",30001)!=0){
		exit_on_error(errno,"failed to connect nynn_daemon for writting!");
	}
	
	Socket listenrso("127.0.0.1",INADDR_ANY);
	struct sockaddr_in saddr;
	socklen_t len=sizeof(saddr);
	listenrso.getsockname(&saddr,&len);
	char host[INET_ADDRSTRLEN];
	listenrso.getlocalhost(host);
	info("sockaddr=%s:%d",host,listenrso.getlocalport());
	if (sizeof(saddr)!=wso.send((char*)&saddr,sizeof(saddr))){
		exit_on_error(errno,"failed to send reading port to nynn_daemon!");
	}
	
	listenrso.listen(1);
	Socket rso=listenrso.accept();
	int val=1;
	if (sizeof(val)!=rso.recv((char*)&val,sizeof(val),MSG_WAITALL)){
		exit_on_error(errno,"failed to connect nynn_daemon for reading!");
	}
	
	info("val=%d",val);
	this->wfd=wso.getsockfd();
	this->rfd=rso.getsockfd();
}

nynn_tap_t::~nynn_tap_t()
{
	close(this->rfd);
	close(this->wfd);
	pthread_mutex_destroy(&this->rlock);
	pthread_mutex_destroy(&this->wlock);
}

int nynn_tap_t::write(uint32_t *inetaddr,size_t num,char*buff,size_t size)
{
	char *shm;
	size_t shmsize=size+sizeof(size);
	int shmid=nynn_shmat(-1,(void**)&shm,shmsize,false);
	memcpy(shm,(char*)&shmsize,sizeof(shmsize));
	memcpy(shm+sizeof(shmsize),buff,size);
	nynn_shmdt(shm);
	
	nynn_token_t req={shmid,shmsize,WRITE,0,NULL};
	
	size_t bufsize=sizeof(req)+sizeof(num)+sizeof(uint32_t)*num;
	char *buf=new char[bufsize];

	memcpy(buf,(char*)&req,sizeof(req));
	memcpy(buf+sizeof(req),(char*)&num,sizeof(num));
	memcpy(buf+sizeof(req)+sizeof(num),(char*)inetaddr,sizeof(uint32_t)*num);
	
	pthread_mutex_lock(&this->wlock);
	Socket so(this->wfd);
	if (bufsize!=so.send(buf,bufsize)){
		error("faild to send");
		pthread_mutex_unlock(&this->wlock);
		return -1;
	}
	pthread_mutex_unlock(&this->wlock);
	return 0;
}


int nynn_tap_t::read(char**buff,size_t*size)
{
	Socket so(this->rfd);
	nynn_token_t tok;	
	tok.nr_cmd=READ;
	pthread_mutex_lock(&this->rlock);
	if (sizeof(tok)!=so.send((char*)&tok,sizeof(tok))){
		error("failed to send");
		return -1;
	}
	if (sizeof(tok)!=so.recv((char*)&tok,sizeof(tok))){
		error("failed to recv");
		return -1;
	}
	pthread_mutex_unlock(&this->rlock);
	
	char *shm;
	if(nynn_shmat(tok.nr_shmid,(void**)&shm,tok.nr_size,true)==-1){
		error("failed to nynn_shmat");
		return -1;
	}
	*size-=sizeof(size_t);
	*buff=new char[*size];
	memcpy(*buff,shm+sizeof(size_t),*size);
	nynn_shmdt(shm);
	return 0;
}

int nynn_write(nynn_tap_t*tap,uint32_t*inetaddr,size_t num,char*buff,size_t len)
{
	return tap->write(inetaddr,num,buff,len);
}

int nynn_read(nynn_tap_t*tap,char**buff,size_t*len)
{
	return tap->read(buff,len);
}
