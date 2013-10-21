#include<nynn_daemon.h>
#include<nynn.h>
#include<public.h>

link_t *links;
hose_t hoses;
size_t links_max;
size_t hose_max;
size_t writer_num;
size_t reader_num;
size_t shmmax;
size_t shmseg;
uint16_t port;
size_t	rcvbuf;
size_t  sndbuf;
enum{
	CFGPATH_SIZE=64
};
char  cfgpath[CFGPATH_SIZE];
size_t linksize;
char hostname[HOSTNAME_SIZE];
uint32_t hostaddr;
token_queue_t rqueue;
task_queue_t wtask_queue;
task_queue_t rtask_queue;
nynn_token_t* delaycleaned; 

shm_manager_t *shmmgr;

void intr_handler(int signo)
{
	exit(signo);
}

int main(int argc,char*argv[])
{
	struct shminfo info;
	shmctl(0,IPC_INFO,(struct shmid_ds*)&info);
	shmmax=info.shmmax;
	shmseg=info.shmseg/2;

	//default config
	links_max=10;
	writer_num=2;
	reader_num=2;
	port=30001;
	memset(cfgpath,0,sizeof(CFGPATH_SIZE));
	strncpy(cfgpath,"network.cfg",CFGPATH_SIZE);
	rcvbuf=shmmax;
	sndbuf=shmmax;

	while(true){
		int ch=getopt(argc,argv,":+l:r:w:f:p:s:R:n:h");
		if(ch=='?'){
			cout<<"nynn_daemon:invalid option -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_daemon [-f file] [-r num] [-w num] [-l num] [-p port] [-s num] [-R num] [-n num]"<<endl;
			cout<<"Try \"nynn_daemon -h for more help\""<<endl;
			exit(0);
		}else if (ch==':'){
			cout<<"nynn_daemon:option requires argument -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_daemon [-f file] [-r num] [-w num] [-l num] [-p port] [-s num] [-R num] [-n num]"<<endl;
			cout<<"Try \"nynn_daemon -h for more help\""<<endl;
			exit(0);
		}else if (ch==-1){
			break;
		}else if (ch=='l'){
			links_max=(atoi(optarg)<=0?links_max:atoi(optarg));
		}else if (ch=='r'){
			reader_num=(atoi(optarg)<=0?reader_num:atoi(optarg));
		}else if (ch=='w'){
			writer_num=(atoi(optarg)<=0?writer_num:atoi(optarg));
		}else if (ch=='f'){
			strncpy(cfgpath,optarg,CFGPATH_SIZE);
		}else if (ch=='s'){
			sndbuf=(atoi(optarg)<0?sndbuf:atoi(optarg));
		}else if (ch=='R'){
			rcvbuf=(atoi(optarg)<0?sndbuf:atoi(optarg));
		}else if (ch=='p'){
			port=(atoi(optarg)<0?port:atoi(optarg));
		}else if (ch=='n'){
			shmseg=(atoi(optarg)<0?shmseg:atoi(optarg));
		}else {
			cout<<"Usage:nynn_daemon [-f file] [-r num] [-w num] [-l num] [-p port] [-s num] [-R num] [-n num]"<<endl;
			cout<<"nynn_daemon is a service for message multicast."<<endl;
			cout<<"Options:"<<endl;
			cout<<"\t-l the maximum number of links(default 10)"<<endl;
			cout<<"\t-r the number of threads for reading message(default 2)"<<endl;
			cout<<"\t-w the number of threads for writing message(default 2)"<<endl;
			cout<<"\t-f the path of network configure file(default './network.cfg')"<<endl;
			cout<<"\t-p the local listen port(default 30001)"<<endl;
			cout<<"\t-s the sending buffer size(default shmmax)"<<endl;
			cout<<"\t-R the receiving buffer size(default shmmax)"<<endl;
			cout<<"\t-n the maximum number of msgs(default shmmax)"<<endl;
			cout<<"\t-h help nynn_daemon"<<endl;
			exit(0);
		}

	}	


	struct sigaction sa_intr;
	sa_intr.sa_handler=intr_handler;
	sigaction(SIGINT,&sa_intr,NULL);

	links=new link_t[links_max];
	shmmgr=new shm_manager_t(shmseg);

	linksize=loadconfig(cfgpath,links,links_max);
	hostaddr=gethostaddr(hostname,HOSTNAME_SIZE);
	info("hostname=%s\n",hostname);
	info("hostaddr=%d\n",hostaddr);
	info("linksize=%d\n",linksize);

	if (linksize>1){
		size_t n=0;
		for (n=0;n<linksize;n++){
			links[n].cachedfragment=NULL;
			pthread_mutex_init(&links[n].wlock,NULL);
			pthread_mutex_init(&links[n].rlock,NULL);
			cout<<links[n].hostname<<":"
				<<links[n].hostaddr<<":"
				<<links[n].port<<endl;
		}
		// links has linksize items
		size_t k=0;
		while(strcmp(links[k].hostname,hostname)!=0 &&
				links[k].hostaddr != hostaddr){
			k++;
		}

		pthread_t *tids =new pthread_t[linksize-k];
		pthread_create(&tids[0],NULL,acceptor,&links[k]);

		for (size_t i=k+1;i<linksize;i++){
			pthread_create(&tids[i-k],NULL,connector,&links[i]);
		}

		for (size_t i=0;i<linksize-k;i++){
			int errnum;
			if ((errnum=pthread_join(tids[i],NULL)!=0)){
				exit_on_error(errnum,"failed to pthread_join");
			}
		}
		delete[] tids;

		delaycleaned=NULL;

		pthread_t poller_tid;
		pthread_create(&poller_tid,NULL,poller,NULL);

		pthread_t *reader_tid=new pthread_t[reader_num];
		for (size_t i=0;i<reader_num;i++){
			pthread_create(reader_tid+i,NULL,reader,NULL);
		}
		pthread_t *writer_tid=new pthread_t[writer_num];
		for (size_t i=0;i<writer_num;i++){
			pthread_create(writer_tid+i,NULL,writer,NULL);
		}
	}

	Socket recipient("127.0.0.1",port);
	recipient.listen(200);

	while(true){
		Socket response=recipient.accept();
		pthread_t tid;
		pthread_create(&tid,NULL,wresponder,(void*)response.getsockfd());
	}
	return 0;
}


bool operator<(const link_t &lhs,const link_t&rhs)
{
	int res=strcmp(lhs.hostname,rhs.hostname);
	if (res<0){
		return true;
	}else if (res>0){
		return false;
	}else{
		return lhs.hostaddr==rhs.hostaddr?
			lhs.port<rhs.port:
			lhs.hostaddr<rhs.hostaddr;
	}
}

int loadconfig(const char*cfgpath,link_t *links,size_t size)
{
	int k=0;
	char line[128];
	char hostname[32],hostaddr[32],port[32],*saveptr;

	fstream cfg_fin(cfgpath);
	do{
		cfg_fin.getline(line,128);
		if(cfg_fin.fail()&&!cfg_fin.eof()){
			cerr<<"line width exceeds 128 bytes"<<endl;
			cerr<<line<<endl;
			return false;
		}
		chop('#',line,line);
		ltrim(" \t",line,line);
		rtrim(" \t",line,line);
		if (strlen(line)!=0){
			strcpy(hostname,strtok_r(line," \t",&saveptr));
			strcpy(hostaddr,strtok_r(NULL," \t",&saveptr));
			strcpy(port,strtok_r(NULL," \t",&saveptr));
			if(k>=size){
				cout<<"to many hosts!"<<endl;
				return false;
			}
			strcpy(links[k].hostname,hostname);
			inet_pton(AF_INET,hostaddr,&links[k].hostaddr);
			links[k].port=htons((uint16_t)atoi(port));
			k++;
		}
	}while(!cfg_fin.eof());
	sort(links,links+k);
	return k;
}

void* connector(void*args)
{
	link_t* link=(link_t*)args;
	sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = link->hostaddr;
	saddr.sin_port = link->port;
	Socket sock;

	while(sock.connect(saddr)!=0){
		sleep(1);
		cout<<"try connect host "<<link->hostname<<"..."<<endl;
	}

	link->rfd=sock.getsockfd();
	setNonBlocking(link->rfd);
	if (setsockopt(link->rfd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))!=0){
		exit_on_error(errno,"failed to set sndbuf!");
	}
	if (setsockopt(link->rfd,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf))!=0){
		exit_on_error(errno,"failed to set rcvbuf!");
	}
	link->wfd=dup(link->rfd);
	info("host:%s rfd:%d wfd:%d",link->hostname,link->rfd,link->wfd);
	char buff[INET_ADDRSTRLEN];
	sock.getlocalhost(buff);
	uint16_t port=sock.getlocalport();
	cout<<"succeeded in connecting host "<<link->hostname
		<<" at "<<buff<<":"<<port<<endl;
	return NULL;
}

void* acceptor(void*args)
{
	link_t* link=(link_t*)args;
	sockaddr_in saddr;
	char buff[INET_ADDRSTRLEN];
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = link->hostaddr;
	saddr.sin_port = link->port;
	Socket listenSock(saddr);
	listenSock.listen(10);
	size_t n=link-links;
	for (size_t i=0;i<n;i++){
		Socket connSock=listenSock.accept();
		uint32_t ip=connSock.getremotehost(buff,INET_ADDRSTRLEN);
		uint16_t port=connSock.getremoteport();
		cout<<"succeeded in accepting connection from "<<buff<<":"<<port<<endl;
		size_t j=0;
		for(j=0;j<n&&links[j].hostaddr!=ip;j++);
		if (j==n){
			exit_on_error(0,"failed to find link item");
		}
		links[j].wfd=connSock.getsockfd();
		if (setsockopt(links[j].wfd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))!=0){
			exit_on_error(errno,"failed to set sndbuf!");
		}
		if (setsockopt(links[j].wfd,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf))!=0){
			exit_on_error(errno,"failed to set rcvbuf!");
		}
		setNonBlocking(links[j].wfd);
		links[j].rfd=dup(links[j].wfd);
		info("host:%s rfd:%d wfd:%d",links[j].hostname,links[j].rfd,links[j].wfd);
	}
	listenSock.close();
	return NULL;
}

void* poller(void* args)
{
	int epollfd,readyfds;
	struct epoll_event ev,*readyEvents,*pev;
	//each link has a socket, each socket has two fds,
	//one for writing,another for reading.
	readyEvents = new epoll_event[linksize*2];

	if ((epollfd=epoll_create(linksize*2))==-1){
		exit_on_error(errno,"failed to call epoll_create");
	}

	for(size_t i=0;i<linksize;i++){
		if (links[i].hostaddr==hostaddr)continue;
		ev.data.fd = links[i].rfd;
		ev.events  = EPOLLIN|EPOLLET;
		epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);
		ev.data.fd = links[i].wfd;
		//ev.events  = EPOLLOUT|EPOLLET;
		ev.events  = EPOLLOUT;
		epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);
	}

	while(true){
		readyfds=epoll_wait(epollfd,readyEvents,linksize*2,-1);
		if (readyfds==-1){
			exit_on_error(errno,"failed to call epoll_wait");
		}

		for (int i=0;i<readyfds;i++){
			pev=readyEvents+i;
			if (pev->events & EPOLLIN){

				// search the ready read socket in links.
				link_t *link=links;
				while(link->rfd!=pev->data.fd)link++;

				task_t *t=new task_t(READ,link->rfd);
				rtask_queue.push(t);

			}else if (pev->events & EPOLLOUT){

				// search the ready write socket in links.
				link_t *link=links;
				while(link->wfd!=pev->data.fd)link++;

				if(link->wqueue.empty())continue;

				task_t *t=new task_t(WRITE,link->wfd);
				wtask_queue.push(t);

			}else if (pev->events & EPOLLERR){
				error("EPOLLERR happens!(fd=%d)",pev->data.fd);
				exit_on_error(0,"nynn exit when EPOLLERR happens!");
			}else{
				warn("can't reach here");
				exit_on_error(0,"unknown error happans!");
			}
		}
	}
}

void*  writer(void* args)
{
	while(true){
		task_t *t=wtask_queue.pop();	
		Socket sock(t->sockfd);

		size_t i=0;
		while(i<linksize && links[i].wfd!=sock.getsockfd())i++;
		link_t *link=&links[i];

		pthread_mutex_t *wlock=&links->wlock;
		int errnum=pthread_mutex_trylock(wlock);
		//other thread is busy using socket
		if(errnum==EBUSY){
			continue;
		}else if (errnum!=0){
			sock.close();
			exit_on_error(errnum,"failed to pthread_mutex_trylock");
		}

		if (!link->wqueue.empty()){
			nynn_token_t*req=link->wqueue.pop();
			//if (--req->refcount==0){
			if (delaycleaned!=NULL){
				shmmgr->release(delaycleaned->shm);
				delete delaycleaned;
			}
			delaycleaned=req;
			//}
			info("SEND(%d)",req->size);
			ssize_t size=0;
			do{
				ssize_t s=sock.send(req->shm+size,req->size-size);
				if(s==-1 && errno!=EAGAIN){
					error("sockfd=%d",sock.getsockfd());
					exit(1);
				}else if(s>0){
					size+=s;
				}else{//s==-1 && errno==EAGAIN
					size+=0;
				}
			}while(size<req->size);
		}
		pthread_mutex_unlock(wlock);
	}
	return NULL;
}

void*  reader(void* args)
{
	char buff[65536];
	while(true){
		task_t *t=rtask_queue.pop();	
		Socket sock(t->sockfd);
		delete t;

		size_t i=0;
		while(i<linksize && links[i].rfd!=sock.getsockfd())i++;
		nynn_token_t **cached=&links[i].cachedfragment;
		pthread_mutex_t *rlock=&links[i].rlock;

		pthread_mutex_lock(rlock);
		ssize_t size;
		do{
			memset(buff,0,sizeof(buff));
			size=sock.recv(buff,sizeof(buff));
			if (size==-1 && errno!=EAGAIN){
				error("error happens when recv!");
				exit(1);
			}else if (size==-1 && errno==EAGAIN){
				continue;
			}else if (size==0){
				error("peer socket is closed!");
				exit(1);
			}

			char *p=buff;
			size_t fragsize;
			size_t msgsize;
			size_t remainsize;

			do{
				if((*cached)==NULL){
					(*cached)=new nynn_token_t;

					size_t s=size-(p-buff);		
					//boundary point breaks message size field into two.
					if (s<sizeof(size_t)){
						(*cached)->size=s;
						(*cached)->shm=new char[sizeof(size_t)];
						memcpy((*cached)->shm,p,s);
						break;
					}
					//boundary point breaks message body.
					msgsize=*(size_t*)p;
					info("msgsize=%d",msgsize);
					(*cached)->shmid=shmmgr->require(-1,(void**)&(*cached)->shm,msgsize ,false);

					if ((*cached)->shmid==-1){
						sock.close();
						exit_on_error(errno,"failed to shmmgr->require");
					}

					(*cached)->size=(s<msgsize?s:msgsize);
					memcpy((*cached)->shm,p,(*cached)->size);
					p+=(*cached)->size;

					//recv a integral msg
					info("msgsize=%d,size=%d",msgsize,(*cached)->size);
					if (msgsize==(*cached)->size){

						info("RECV(shmid=%d size=%d)"
								,(*cached)->shmid
								,(*cached)->size);

						nynn_msg_t *msg=(nynn_msg_t*)(*cached)->shm;
						token_queue_t *rqueue=hoses.get(msg->msghdr.msgid);
						shmmgr->release((*cached)->shm);
						if (rqueue!=NULL)rqueue->push((*cached));
						else nynn_shmrm((*cached)->shmid);
						(*cached)=NULL;
						//handle next msg
						continue;
					}else{
						info("finish handling buff!",pthread_self());
						//finish handling buff
						break;
					}
				}else{ 

					fragsize=(*cached)->size;
					info("fragsize=%d",fragsize);
					size_t s=size-(p-buff);
					//boundary point breaks message size field into two.
					//so can't get size of msg.
					if (fragsize+s<sizeof(size_t)){
						memcpy((*cached)->shm+fragsize,p,s);	
						(*cached)->size+=s;
						break;
					}
					//can get size of msg
					if (fragsize<sizeof(size_t)&&fragsize+s>=sizeof(size_t)){
						size_t s1=sizeof(size_t)-fragsize;
						memcpy((*cached)->shm+fragsize,p,s1);	
						p+=s1;
						msgsize=*(size_t*)(*cached)->shm;
						delete (*cached)->shm;

						(*cached)->shmid=shmmgr->require(-1,(void**)&(*cached)->shm,msgsize,false);

						if ((*cached)->shmid==-1){
							sock.close();
							exit_on_error(errno,"failed to shmmgr->require");
						}
						memcpy((*cached)->shm,&msgsize,sizeof(size_t));
						(*cached)->size=sizeof(size_t);
					}
					//handle incomplete msg
					s=size-(p-buff);
					msgsize=*(size_t*)(*cached)->shm;
					fragsize=(*cached)->size;
					remainsize=msgsize-fragsize;
					if (remainsize<=s){
						(*cached)->size=msgsize;
						memcpy((*cached)->shm+fragsize,p,remainsize);
						p+=remainsize;
						info("RECV(shmid=%d size=%d):%s"
								,(*cached)->shmid
								,(*cached)->size
								,(*cached)->shm+sizeof(size_t));

						nynn_msg_t *msg=(nynn_msg_t*)(*cached)->shm;
						token_queue_t *rqueue=hoses.get(msg->msghdr.msgid);
						shmmgr->release((*cached)->shm);
						if (rqueue!=NULL)rqueue->push((*cached));
						else nynn_shmrm((*cached)->shmid);
						(*cached)=NULL;
					}else{
						(*cached)->size+=s;
						memcpy((*cached)->shm+fragsize,p,s);
						break;
					}	
				}			
			}while(size-(p-buff)>0);
		}while(size==65536);
		pthread_mutex_unlock(rlock);
	}
}

void* wresponder(void*args)
{
	Socket wresponse((intptr_t)args);

	if (pthread_detach(pthread_self())!=0){
		wresponse.close();
		thread_exit_on_error(errno,"failed to pthread_detach");
	}

	struct sockaddr_in saddr;
	if (sizeof(saddr)!=wresponse.recv((char*)&saddr,sizeof(saddr),MSG_WAITALL)){
		wresponse.close();
		thread_exit_on_error(errno,"failed to recv client's port!");	
	}
	Socket rresponse;
	if (rresponse.connect(saddr)!=0){
		error("failed to connect");
		pthread_exit((void*)1);
	};
	pthread_t tid;
	pthread_create(&tid,NULL,rresponder,(void*)rresponse.getsockfd());


	while(true){
		nynn_token_t *req=new nynn_token_t;
		if (sizeof(*req)!=wresponse.recv((char*)req,sizeof(*req),MSG_WAITALL)){
			wresponse.close();
			pthread_cancel(tid);
			thread_exit_on_error(errno,"terminate thread!");
		}


		uint32_t ntohlhost=ntohl(req->host);
		if (req->host==hostaddr||0x7f000000<=ntohlhost&&ntohlhost<=0x7fffffff){
			shmmgr->require(req->shmid,(void**)&req->shm,req->size,false);
			nynn_msg_t *msg=(nynn_msg_t*)req->shm;
			req->cmd=READ;
			token_queue_t*rqueue=hoses.get(msg->msghdr.msgid);
			shmmgr->release(req->shm);
			if (rqueue!=NULL)rqueue->push(req);
			else nynn_shmrm(req->shmid);
			continue;
		}	

		shmmgr->require(req->shmid,(void**)&req->shm,req->size,true);
		size_t j=0;
		while(j<linksize && links[j].hostaddr!=req->host)j++;
		if (j<linksize){
			info("push req to %s",links[j].hostname);
			links[j].wqueue.push(req);
		}
		info("WRITE(%d)",req->size);
	}
	return NULL;
}

void* rresponder(void*args)
{
	Socket rresponse((intptr_t)args);

	if (pthread_detach(pthread_self())!=0){
		rresponse.close();
		thread_exit_on_error(errno,"failed to pthread_detach");
	}
	if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL)!=0)
	{
		rresponse.close();
		thread_exit_on_error(errno,"failed to pthread_setcanceltype");
	}
	char msgid[MSGIDSIZE];
	memset(msgid,0,MSGIDSIZE);

	if (sizeof(msgid)!=rresponse.recv(msgid,sizeof(msgid),MSG_WAITALL)){
		rresponse.close();
		thread_exit_on_error(errno,"failed to abtain hostno.");
	}
	if (!(strlen(msgid)<MSGIDSIZE)){
		rresponse.close();
		thread_exit_on_error(errno,"msgid exceeds its maximum size");
	}

	if (hoses.get(msgid)!=NULL){
		rresponse.close();
		thread_exit_on_error(EINVAL,"the same msgid registered duplicately.");
	}

	token_queue_t *rqueue=hoses.add(msgid);

	if (sizeof(size_t)!=rresponse.send((char*)&shmmax,sizeof(size_t),MSG_NOSIGNAL)){
		rresponse.close();
		thread_exit_on_error(errno,"failed to pthread_detach");
	}

	release_rqueue_t release(&hoses,msgid,rqueue);
	while(true){
		nynn_token_t *req=new nynn_token_t;
		if (sizeof(*req)!=rresponse.recv((char*)req,sizeof(*req),MSG_WAITALL)){
			rresponse.close();
			thread_exit_on_error(errno,"terminate thread!");
		}

		info("READ");
		req=rqueue->pop();
		info("READ(shmid=%d size=%d)",req->shmid,req->size);
		if (sizeof(*req)!=rresponse.send((char*)req,sizeof(*req),MSG_NOSIGNAL)){
			rresponse.close();
			thread_exit_on_error(errno,"terminate thread!");
		}
		delete req;
	}
	return NULL;
}
