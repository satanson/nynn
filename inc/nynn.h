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
	CONTROL=2
};

struct nynn_token_t{
	int 	nr_shmid;
	size_t 	nr_size;
	int 	nr_cmd;
	int 	nr_refcount;
	char*  	nr_shm;
};
class nynn_tap_t{
	private:
		pthread_mutex_t wlock;
		pthread_mutex_t rlock;
		int wfd;
		int rfd;
		size_t shmmax;
	public:
		nynn_tap_t();
		~nynn_tap_t();

		int read(char**buff,size_t *size);
		int write(uint32_t *inetaddr, size_t num, char*buff,size_t size);
};

int nynn_shmat(int shmid, void**shmaddr, size_t size,bool removal);
int nynn_shmdt(const void*shmaddr);

int nynn_write(nynn_tap_t *tap,uint32_t *inetaddr,size_t num, char*buff,size_t size);
int nynn_read(nynn_tap_t *tap,char**buff,size_t *size);
#endif
