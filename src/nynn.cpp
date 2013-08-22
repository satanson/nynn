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

//write request format
//----------------------
//|---nynn_token_t---|
//|-------num----------|
//|------uint32_t(1)---|
//|------uint32_t(2)---|
//|------........------|
//|------uint32_t(num)-|
//----------------------

int nynn_write(uint32_t *inetaddr,size_t num,nynn_token_t* req)
{
	Socket so;
	if (so.connect("127.0.0.1",30001)!=0){
		error("failed to connect");
		return -1;
	}

	size_t bufsize=sizeof(*req)+sizeof(num)+sizeof(uint32_t)*num;
	char *buf=new char[bufsize];

	memcpy(buf,(char*)req,sizeof(*req));
	memcpy(buf+sizeof(*req),(char*)&num,sizeof(num));
	memcpy(buf+sizeof(*req)+sizeof(num),(char*)inetaddr,sizeof(uint32_t)*num);
	//fix
	if (bufsize!=so.send(buf,bufsize)){
		error("faild to send");
		so.close();
		return -1;
	}
	so.close();
	return 0;
}
int nynn_write(uint32_t *inetaddr, size_t num, int shmid,size_t size)
{
	nynn_token_t req={shmid,size,WRITE,0,NULL};
	return nynn_write(inetaddr,num,&req);
}
//msg format
//---------------------
//|------msgsize------|
//|------msg----------|
//---------------------
int nynn_write(uint32_t *inetaddr, size_t num, char*buff,size_t size)
{
	char *shm;
	size_t shmsize=size+sizeof(size);
	int shmid=nynn_shmat(-1,(void**)&shm,shmsize,false);
	memcpy(shm,(char*)&shmsize,sizeof(shmsize));
	memcpy(shm+sizeof(shmsize),buff,size);

	nynn_write(inetaddr,num,shmid,shmsize);
	nynn_shmdt(shm);
}

int nynn_read(nynn_token_t* req)
{
	Socket so;
	if (so.connect("127.0.0.1",30001)!=0){
		error("failed to connect");
		return -1;
	}
	req->nr_cmd=READ;
	if (sizeof(*req)!=so.send((char*)req,sizeof(*req))){
		error("failed to send");
		so.close();
		return -1;
	}
	if (sizeof(*req)!=so.recv((char*)req,sizeof(*req))){
		error("failed to recv");
		so.close();
		return -1;
	}
	so.close();
	return 0;
}
int nynn_read(int *shmid,size_t *size)
{
	nynn_token_t req;
	if (nynn_read(&req)==0){
		*shmid=req.nr_shmid;
		*size=req.nr_size;
		return 0;
	}else{
		return -1;
	}
}
int nynn_read(char**buff,size_t *size){
	int shmid;
	char*shm;
	if (nynn_read(&shmid,size)==0){
		if(nynn_shmat(shmid,(void**)&shm,*size,true)==-1){
			error("failed to nynn_shmat");
			return -1;
		}
		*size-=sizeof(size_t);
		*buff=new char[*size];
		memcpy(*buff,shm+sizeof(size_t),*size);
		nynn_shmdt(shm);
		return 0;
	}
	return -1;
}
