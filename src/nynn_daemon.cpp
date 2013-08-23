#include<nynn_daemon.h>
#include<nynn.h>
#include<public.h>

link_t links[LINK_MAX];
size_t linksize;
char hostname[HOSTNAME_SIZE];
uint32_t hostaddr;
concurrent_queue<nynn_token_t*> rqueue;
concurrent_queue<task_t*> task_queue;
nynn_token_t* delaycleaned; 

void intr_handler(int signo)
{
	exit(signo);
}

int main(int argc,char*argv[])
{
	struct sigaction sa_intr;
	sa_intr.sa_handler=intr_handler;
	sigaction(SIGINT,&sa_intr,NULL);
	const char* cfgpath=argv[1];
	linksize=loadconfig(cfgpath,links,LINK_MAX);
	hostaddr=gethostaddr(hostname,HOSTNAME_SIZE);
	info("hostname=%s\n",hostname);
	info("hostaddr=%d\n",hostaddr);
	info("linksize=%d\n",linksize);

	size_t n=0;
	for (n=0;n<linksize;n++){
		links[n].cachedfragment=NULL;
		pthread_mutex_init(&links[n].wlock,NULL);
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
	pthread_create(&tids[0],NULL,accept_handler,&links[k]);

	for (size_t i=k+1;i<linksize;i++){
		pthread_create(&tids[i-k],NULL,connect_handler,&links[i]);
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
	pthread_t exchanger_tids[10];

	for (size_t i=0;i<3;i++){
		pthread_create(exchanger_tids+i,NULL,exchanger,NULL);
	}

	Socket recipient("127.0.0.1",30001);
	recipient.listen(200);

	while(true){
		Socket response=recipient.accept();
		nynn_token_t *req=new nynn_token_t;
		if (sizeof(*req)!=response.recv((char*)req,sizeof(*req))){
			error("failed to recv request!");
		}

		switch(req->nr_cmd){
			case WRITE:
				{
					nynn_shmat(req->nr_shmid,(void**)&req->nr_shm,req->nr_size,true);
					size_t num=0;
					response.recv((char*)&num,sizeof(num)); 
					uint32_t *inetaddr=new uint32_t[num];
					response.recv((char*)inetaddr,sizeof(uint32_t)*num);
					req->nr_refcount=num;

					for (size_t i=0;i<num;i++){
						size_t j=0;
						while(j<linksize && links[j].hostaddr!=inetaddr[i])j++;
						if (j<linksize){
							info("push req to %s",links[j].hostname);
							links[j].wqueue.push(req);
						}
					}
					info("WRITE(%d): %s",req->nr_size,req->nr_shm+sizeof(size_t));
				}
				break;

			case READ:
				{
					info("READ");
					req=rqueue.pop();
					info("READ(shmid=%d size=%d)",req->nr_shmid,req->nr_size);
					response.send((char*)req,sizeof(*req));
					delete req;
				}
				break;

			case CONTROL:
				break;
			default:
				error("never reach here");
				break;
		}
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

void* connect_handler(void*args)
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
	link->wfd=dup(link->rfd);
	info("host:%s rfd:%d wfd:%d",link->hostname,link->rfd,link->wfd);
	char buff[INET_ADDRSTRLEN];
	sock.getlocalhost(buff);
	uint16_t port=sock.getlocalport();
	cout<<"succeeded in connecting host "<<link->hostname
		<<" at "<<buff<<":"<<port<<endl;
	return NULL;
}

void* accept_handler(void*args)
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
		setNonBlocking(links[j].wfd);
		links[j].rfd=dup(links[j].wfd);
		info("host:%s rfd:%d wfd:%d",links[j].hostname,links[j].rfd,links[j].wfd);
	}
	listenSock.close();
	return NULL;
}

void* poller(void* arg)
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
				task_queue.push(t);

			}else if (pev->events & EPOLLOUT){

				// search the ready write socket in links.
				link_t *link=links;
				while(link->wfd!=pev->data.fd)link++;

				//if(link->wqueue.empty())break;

				task_t *t=new task_t(WRITE,link->wfd);
				task_queue.push(t);

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

void*  exchanger(void* arg)
{
	char buff[65536];
	while(true){
		task_t *t=task_queue.pop();	
		Socket sock(t->sockfd);
		int tasktype=t->tasktype;
		delete t;

		if (tasktype==WRITE){


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
				if (--req->nr_refcount==0){
					if (delaycleaned!=NULL){
						nynn_shmdt(delaycleaned->nr_shm);
						delete delaycleaned;
					}
					delaycleaned=req;
				}
				info("SEND:%s",req->nr_shm+sizeof(size_t));
				ssize_t size=0;
				do{
					size_t s=sock.send(req->nr_shm+size,req->nr_size-size);
					if(s==-1 && errno!=EAGAIN){
						error("sockfd=%d",sock.getsockfd());
						exit(1);
					}else if(s>0){
						size+=s;
					}else{//s==-1 && errno==EAGAIN
						size+=0;
					}
				}while(size<req->nr_size);
			}
			pthread_mutex_unlock(wlock);

		}else if (tasktype==READ){
			size_t i=0;
			while(i<linksize && links[i].rfd!=sock.getsockfd())i++;
			nynn_token_t **cached=&links[i].cachedfragment;
			

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
				}else{
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
								(*cached)->nr_size=s;
								(*cached)->nr_shm=new char[sizeof(size_t)];
								memcpy((*cached)->nr_shm,p,s);
								break;
							}
							//boundary point breaks message body.
							msgsize=*(size_t*)p;
							info("msgsize=%d",msgsize);
							(*cached)->nr_shmid=nynn_shmat(-1,
									(void**)&(*cached)->nr_shm,msgsize ,false);

							if ((*cached)->nr_shmid==-1){
								sock.close();
								exit_on_error(errno,"failed to nynn_shmat");
							}

							(*cached)->nr_size=(s<msgsize?s:msgsize);
							memcpy((*cached)->nr_shm,p,(*cached)->nr_size);
							p+=(*cached)->nr_size;

							//recv a integral msg
							info("msgsize=%d,nr_size=%d",msgsize,(*cached)->nr_size);
							if (msgsize==(*cached)->nr_size){

								info("RECV(shmid=%d size=%d):%s"
										,(*cached)->nr_shmid
										,(*cached)->nr_size
										,(*cached)->nr_shm+sizeof(size_t));

								nynn_shmdt((*cached)->nr_shm);
								rqueue.push((*cached));
								(*cached)=NULL;
								//handle next msg
								continue;
							}else{
								info("finish handling buff!");
								//finish handling buff
								break;
							}
						}else{ 

							fragsize=(*cached)->nr_size;
							info("fragsize=%d",fragsize);
							size_t s=size-(p-buff);
							//boundary point breaks message size field into two.
							//so can't get size of msg.
							if (fragsize+s<sizeof(size_t)){
								memcpy((*cached)->nr_shm+fragsize,p,s);	
								(*cached)->nr_size+=s;
								break;
							}
							//can get size of msg
							if (fragsize<sizeof(size_t)&&fragsize+s>=sizeof(size_t)){
								size_t s1=sizeof(size_t)-fragsize;
								memcpy((*cached)->nr_shm+fragsize,p,s1);	
								p+=s1;
								msgsize=*(size_t*)(*cached)->nr_shm;
								delete (*cached)->nr_shm;

								(*cached)->nr_shmid=nynn_shmat(-1
										,(void**)&(*cached)->nr_shm,msgsize,false);

								if ((*cached)->nr_shmid==-1){
									sock.close();
									exit_on_error(errno,"failed to nynn_shmat");
								}
								memcpy((*cached)->nr_shm,&msgsize,sizeof(size_t));
								(*cached)->nr_size=sizeof(size_t);
							}
							//handle incomplete msg
							s=size-(p-buff);
							msgsize=*(size_t*)(*cached)->nr_shm;
							fragsize=(*cached)->nr_size;
							remainsize=msgsize-fragsize;
							if (remainsize<s){
								(*cached)->nr_size=msgsize;
								memcpy((*cached)->nr_shm+fragsize,p,remainsize);
								p+=remainsize;
								info("RECV(shmid=%d size=%d):%s"
										,(*cached)->nr_shmid
										,(*cached)->nr_size
										,(*cached)->nr_shm+sizeof(size_t));

								nynn_shmdt((*cached)->nr_shm);
								rqueue.push((*cached));
								(*cached)=NULL;
							}else{
								(*cached)->nr_size+=s;
								memcpy((*cached)->nr_shm+fragsize,p,s);
								break;
							}	
						}			
					}while(size-(p-buff)>0);
				}
			}while(size==65536);

		}else{
			warn("can't reach here");
		}
	}
}
