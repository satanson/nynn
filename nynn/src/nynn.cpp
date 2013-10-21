#include<public.h>
#include<nynn.h>
#include<socket.h>
int nynn_shmat(int shmid, void **shmaddr,size_t size,bool removal)
{
	if(shmid==-1){
		shmid=shmget(IPC_PRIVATE,size,IPC_CREAT|IPC_EXCL|0777);
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

int nynn_shmrm(int shmid)
{
	struct shmid_ds ds;
	if (shmctl(shmid,IPC_STAT,&ds)==-1){
		error("failed to shmctl(shmid,IPC_STAT...");
		return -1;
	}
	if (ds.shm_nattch!=0)return 0;
	if (shmctl(shmid,IPC_RMID,NULL)==-1){
		error("failed to remove shared memory");
		return -1;
	}
	return 0;
}

nynn_tap_t::nynn_tap_t(const char* msgid,uint16_t port)
{
	assert(strlen(msgid)<=MSGIDSIZE);	
	pthread_mutex_init(&this->wlock,NULL);
	pthread_mutex_init(&this->rlock,NULL);
	Socket wso;
	if (wso.connect("127.0.0.1",port)!=0){
		exit_on_error(errno,"failed to connect nynn_daemon for writting!");
	}
	
	Socket listenrso("127.0.0.1",INADDR_ANY);
	listenrso.listen(1);
	
	struct sockaddr_in saddr;
	socklen_t len=sizeof(saddr);
	listenrso.getsockname(&saddr,&len);
	char host[INET_ADDRSTRLEN];
	listenrso.getlocalhost(host);
	info("sockaddr=%s:%d",host,listenrso.getlocalport());
	if (sizeof(saddr)!=wso.send((char*)&saddr,sizeof(saddr))){
		exit_on_error(errno,"failed to send reading port to nynn_daemon!");
	}
	
	Socket rso=listenrso.accept();
	char msgid_buf[MSGIDSIZE];
	memset(msgid_buf,0,MSGIDSIZE);
	strcpy(msgid_buf,msgid);

	if (MSGIDSIZE!=rso.send(msgid_buf,MSGIDSIZE)){
		exit_on_error(errno,"failed to send msgid to nynn_daemon!");
	}
	
	if (sizeof(size_t)!=rso.recv((char*)&this->shmmax,sizeof(size_t),MSG_WAITALL)){
		exit_on_error(errno,"failed to connect nynn_daemon for reading!");
	}
	
	info("shmmax=%d",this->shmmax);
	this->wfd=wso.getsockfd();
	this->rfd=rso.getsockfd();
}

/*
nynn_tap_t::nynn_tap_t(uint16_t port,int hoseno)
{
	this->hoseno=hoseno;
	pthread_mutex_init(&this->wlock,NULL);
	pthread_mutex_init(&this->rlock,NULL);
	Socket wso;
	if (wso.connect("127.0.0.1",port)!=0){
		exit_on_error(errno,"failed to connect nynn_daemon for writting!");
	}
	
	Socket listenrso("127.0.0.1",INADDR_ANY);
	listenrso.listen(1);
	
	struct sockaddr_in saddr;
	socklen_t len=sizeof(saddr);
	listenrso.getsockname(&saddr,&len);
	char host[INET_ADDRSTRLEN];
	listenrso.getlocalhost(host);
	info("sockaddr=%s:%d",host,listenrso.getlocalport());
	if (sizeof(saddr)!=wso.send((char*)&saddr,sizeof(saddr))){
		exit_on_error(errno,"failed to send reading port to nynn_daemon!");
	}
	
	Socket rso=listenrso.accept();
	
	if (sizeof(this->hoseno)!=rso.send((char*)&this->hoseno,sizeof(this->hoseno))){
		exit_on_error(errno,"failed to send hoseno number to nynn_daemon!");
	}
	
	if (sizeof(size_t)!=rso.recv((char*)&this->shmmax,sizeof(size_t),MSG_WAITALL)){
		exit_on_error(errno,"failed to connect nynn_daemon for reading!");
	}
	
	info("shmmax=%d",this->shmmax);
	this->wfd=wso.getsockfd();
	this->rfd=rso.getsockfd();
}
*/

nynn_tap_t::~nynn_tap_t()
{
	close(this->rfd);
	close(this->wfd);
	pthread_mutex_destroy(&this->rlock);
	pthread_mutex_destroy(&this->wlock);
}


int nynn_tap_t::infuse(const char*host,const char*msgid,const char*msgbdy,size_t msgbdysize)
{
	assert(strlen(host)<MSGHOSTSIZE);
	assert(strlen(msgid)<MSGIDSIZE);

	char *shm;
	size_t shmsize=sizeof(size_t)+MSGHOSTSIZE+MSGIDSIZE+msgbdysize;
	
	if(shmsize>shmmax){
		error("too big message(max=%d)",this->shmmax);
		return -1;
	}
	int shmid=nynn_shmat(-1,(void**)&shm,shmsize,false);
	if(shmid==-1){
		error("failed to nynn_shmat!");
		return -1;
	}

	nynn_msg_t *msg=(nynn_msg_t*)shm;
	msg->msghdr.msgsize=shmsize;
	strcpy(msg->msghdr.host,host);
	strcpy(msg->msghdr.msgid,msgid);
	memcpy(msg->msgbdy,msgbdy,msgbdysize);

	nynn_shmdt(shm);
	
	nynn_token_t req={shmid,shmsize,WRITE,0,NULL};
	if (inet_pton(AF_INET,host,&req.host)!=1){
		error("failed to convert a string to AF_INET address");
	}

	pthread_mutex_lock(&this->wlock);
	Socket so(this->wfd);
	if (sizeof(req)!=so.send((char*)&req,sizeof(req))){
		error("faild to send");
		pthread_mutex_unlock(&this->wlock);
		return -1;
	}
	pthread_mutex_unlock(&this->wlock);
	return 0;
}

int nynn_tap_t::infuse(const char *msg,size_t msgsize)
{
	const char*host=msg;
	const char*msgid=msg+MSGHOSTSIZE;
	const char*msgbdy=msgid+MSGIDSIZE;
	int msgbdysize=msgsize-MSGHOSTSIZE-MSGIDSIZE;
	return infuse(host,msgid,msgbdy,msgbdysize);
}

/*
int nynn_tap_t::write(uint32_t *inetaddr,size_t num,char*buff,size_t size)
{
	char *shm;
	size_t shmsize=size+sizeof(nynn_msg_t);
	if(shmsize>shmmax){
		info("too big message(max=%d)",this->shmmax);
		return -1;
	}
	int shmid=nynn_shmat(-1,(void**)&shm,shmsize,false);
	nynn_msg_t *msg=(nynn_msg_t*)shm;
	msg->msgsize=shmsize;
	msg->hoseno=this->hoseno;
	memcpy(shm+sizeof(nynn_msg_t),buff,size);
	nynn_shmdt(shm);

	nynn_token_t req={shmid,shmsize,WRITE,0,this->hoseno,NULL};
	
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
*/

int nynn_tap_t::effuse(char**msgbdy,size_t *msgbdysize)
{
	Socket so(this->rfd);
	nynn_token_t tok;	
	tok.cmd=READ;
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
	if(nynn_shmat(tok.shmid,(void**)&shm,tok.size,true)==-1){
		error("failed to nynn_shmat");
		return -1;
	}
	nynn_msg_t *msg=(nynn_msg_t*)shm;
	*msgbdysize=msg->msghdr.msgsize-sizeof(msg->msghdr);
	*msgbdy=new char[*msgbdysize];
	memcpy(*msgbdy,msg->msgbdy,*msgbdysize);
	nynn_shmdt(shm);
	return 0;
}
/*
int nynn_tap_t::read(char**buff,size_t*size)
{
	Socket so(this->rfd);
	nynn_token_t tok;	
	tok.cmd=READ;
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
	if(nynn_shmat(tok.shmid,(void**)&shm,tok.size,true)==-1){
		error("failed to nynn_shmat");
		return -1;
	}
	nynn_msg_t *msg=(nynn_msg_t*)shm;
	*size=msg->msgsize-sizeof(nynn_msg_t);
	*buff=new char[*size];
	memcpy(*buff,shm+sizeof(nynn_msg_t),*size);
	nynn_shmdt(shm);
	return 0;
}
*/
/*
int nynn_write(nynn_tap_t*tap,uint32_t*inetaddr,size_t num,char*buff,size_t len)
{
	return tap->write(inetaddr,num,buff,len);
}
*/

/*
int nynn_read(nynn_tap_t*tap,char**buff,size_t*len)
{
	return tap->read(buff,len);
}
*/
int tap_infuse(nynn_tap_t *tap,const char*msg,size_t msgsize)
{
	return tap->infuse(msg,msgsize);
}

int tap_effuse(nynn_tap_t *tap,char**msgbdy,size_t *msgbdysize)
{
	return tap->effuse(msgbdy,msgbdysize);
}
