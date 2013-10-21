#ifndef NYNN_H_BY_SATANSON
#define NYNN_H_BY_SATANSON
#include<public.h>

typedef unsigned char byte;

class inet4addr{
	typedef union{
		int addr;
		byte b[4];
	}inet4addr_t;

	private:
		inet4addr_t ip;
	public:
		inet4addr(byte b1,byte b2,byte b3,byte b4);
		explicit inet4addr(byte b[4]);
		explicit inet4addr(int addr);
		explicit inet4addr(const char*addr);
		int getaddr();
};

enum{
	WRITE=0,
	READ=1,
	CONTROL=2,
	MSGIDSIZE=40,
	MSGHOSTSIZE=16
};

struct nynn_token_t{
	int 	shmid;
	size_t 	size;
	int 	cmd;
	uint32_t host;
	char*  	shm;
};

class nynn_tap_t{
	private:
		pthread_mutex_t wlock;
		pthread_mutex_t rlock;
		int wfd;
		int rfd;
		int hoseno;
		size_t shmmax;
		char msgid[MSGIDSIZE];
	public:
		//nynn_tap_t(uint16_t port,int hoseno);
		nynn_tap_t(const char*msgid,uint16_t port=30001);
		~nynn_tap_t();

		//int read(char**buff,size_t *size);
		//int write(uint32_t *inetaddr, size_t num, char*buff,size_t size);
		int infuse(const char*host,const char*msgid,const char*msgbdy,size_t msgbdysize);
		int infuse(const char*msg,size_t msgsize);
		int effuse(char**msgbdy,size_t *msgbdysize);
};
struct nynn_msg_t{
	struct {
	size_t msgsize;
    char   host[MSGHOSTSIZE];
	char   msgid[MSGIDSIZE];
	}msghdr;
	char   msgbdy[1];
};
int nynn_shmat(int shmid, void**shmaddr, size_t size,bool removal);
int nynn_shmdt(const void*shmaddr);
int nynn_shmrm(int shmid);
//int nynn_write(nynn_tap_t *tap,uint32_t *inetaddr,size_t num, char*buff,size_t size);
int tap_infuse(nynn_tap_t *tap,const char*msg,size_t msgsize);
//int nynn_read(nynn_tap_t *tap,char**buff,size_t *size);
int tap_effuse(nynn_tap_t *tap,const char**msgbdy,size_t *msgbdysize);

#endif
