#ifndef NYNN_MM_COMMON_BY_SATANSON
#define NYNN_MM_COMMON_BY_SATANSON

#include<iostream>
#include<sstream>
#include<memory>
#include<string>

#include<cstring>
#include<cstdlib>
#include<cerrno>
#include<cstdarg>
#include<cassert>
#include<cstdio>
#include<ctime>
#include<csignal>

#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/mman.h>
#include<sys/shm.h>
#include<sys/sem.h>

#include<unistd.h>
#include<fcntl.h>
#include<execinfo.h>
#include<glob.h>
using namespace std;

typedef unsigned long long int uint64_t;
typedef long long int int64_t;
typedef unsigned short int uint16_t;
typedef short int int16_t;
typedef unsigned char uint8_t;

namespace nynn{ namespace mm{ namespace common{
//declaration
enum{
	ERR_BUFF_SIZE=128,
	ERR_MSG_RESERVED_SIZE=256,
	BACKTRACE_FRAME_SIZE=20,
	VSNPRINTF_BUFF_SIZE=512,

	//log level
	LOG_INFO=0,
	LOG_WARN=1,
	LOG_ERROR=2,
	LOG_ASSERT=3,
	LOG_DEBUG=4,
	//_tag
	INVALID_TAG=0xffffffff,
	LOCK_TAG=0xdeadbeef,
	RWLOCK_TAG=0xfacefeed,
	SHM_TAG=0xadeaddad,

	SEMGET_TRY_MAX=1000,
	//rwlock type
	RWLOCK_SHARED=1,
	RWLOCK_EXCLUSIVE=0,

	DUMMY
};

template <typename T> class resetable_auto_ptr;

class nynn_error_t;
class mmap_file_t;

struct shm_allocator_t;
class lockop_t;
class shm_t;

class flock_t;
class frlock_t;
class fwlock_t;

class raii_flock_t;


string& strerr(int errnum,string& s);
void vlog(ostream &out,const char *file,const int line,
		const char *function,const int level,int errnum,const char *fmt,va_list ap);
void log_debug(ostream&out,const char*file,const int line,
		const char *function,const int level,int errnum,const char *fmt,...);
void log(ostream& out,const char*file,const int line,
		const char *function,const int level,int errnum,const char *fmt,...);
int rand_int();
bool file_exist(const string& path);

#define log_i(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_INFO,0,(msg),##__VA_ARGS__)
#define log_w(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_WARN,0,(msg),##__VA_ARGS__)
#define log_e(errnum) \
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_ERROR,errnum,NULL)
#define log_a(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_ASSERT,0,(msg),##__VA_ARGS__)
#define log_d(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_DEBUG,0,(msg),##__VA_ARGS__)

#define THROW_ERROR \
   	throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__)
#define THROW_ERROR_WITH_ERRNO(errnum) \
   	throw nynn_error_t((errnum),__FILE__,__LINE__,__FUNCTION__)

bool file_exist(const string& path)
{
	struct stat st;
	if (stat(path.c_str(),&st)==0)return true;
	if (errno!=ENOENT)log_e(errno);
	return false;
}

//definition
template <typename T> class resetable_auto_ptr{
public:
	resetable_auto_ptr(T* p):_ptr(p) {}
	~resetable_auto_ptr(){ if (_ptr) delete _ptr;}
	T* operator->(){ return _ptr;}
	void reset() {_ptr=NULL;}
private:
	T *_ptr;
	resetable_auto_ptr(const resetable_auto_ptr&);
	resetable_auto_ptr& operator=(const resetable_auto_ptr&);

};

class nynn_error_t{
public:
	nynn_error_t(const int errnum,const char* file
			,const int line,const char*function)
		:ne_errnum(errnum),ne_file(file),ne_line(line),ne_function(function)
	{
		stringstream pack;
		string s;
		pack<<"####ERROR@"
			<<ne_file
			<<"#"<<ne_line
			<<"-"<<ne_function<<"():"
				<<"["<<ne_errnum<<"]"
					   <<strerr(errnum,s);

		ne_msg.reserve(ERR_MSG_RESERVED_SIZE);
		ne_msg+=pack.str();
		ne_framesize=backtrace(ne_frames,BACKTRACE_FRAME_SIZE);
		print_backtrace();
	}

	const char* get_error()const
	{
		return ne_msg.c_str();
	}

	void print_backtrace(){
		cerr<<ne_msg<<endl;
		char **frames=backtrace_symbols(ne_frames,ne_framesize);
		cerr<<"backtrace:"<<endl;
		if (frames!=NULL) {
			for (size_t i=0;i<ne_framesize;i++)
				cerr<<frames[i]<<endl;
		}
		free(frames);
	}

private:
	const int 	ne_errnum;
	const char* ne_file;
	const int 	ne_line;
	const char* ne_function;
	string 		ne_msg;
	void* 		ne_frames[BACKTRACE_FRAME_SIZE];
	size_t 		ne_framesize;
};

class mmap_file_t{
public:
	//create a shared mapping file for already existed file
	explicit mmap_file_t(const string& path) throw(nynn_error_t)
		:_path(path),_offset(0),_base(0)
	{
		_fd=open(path.c_str(),O_RDWR);
		if (_fd<0)
			THROW_ERROR;

		_length=lseek(_fd,0,SEEK_END);
		if (_length==-1)
			THROW_ERROR;

		_base=mmap(NULL,_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,_fd,_offset);

		if (_base==MAP_FAILED)
			THROW_ERROR;

		if (close(_fd)!=0)		
			THROW_ERROR;
	}

	//create a shared mapping file for already existed file
	mmap_file_t(const string& path,size_t length,off_t offset)throw(nynn_error_t)
		:_path(path),_offset(offset),_base(0)
	{
		_fd=open(path.c_str(),O_RDWR);
		if (_fd<0)
			THROW_ERROR;

		_length=lseek(_fd,0,SEEK_END);
		if (_length==-1)
			THROW_ERROR;

		_length=_offset+length<_length?length:_length-_offset;

		_base=mmap(NULL,_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,_fd,_offset);
		if (_base==MAP_FAILED)
			THROW_ERROR;

		if (close(_fd)!=0)		
			THROW_ERROR;

	}

	// create a private mapping file for a new file
	mmap_file_t(const string& path,size_t length)throw(nynn_error_t)
		:_path(path),_length(length),_offset(0),_base(0)
	{
		_fd=open(path.c_str(),O_RDWR|O_CREAT|O_EXCL);
		if (_fd<0)
			THROW_ERROR;

		int PAGESIZE=sysconf(_SC_PAGESIZE);
		_length=_length<PAGESIZE?PAGESIZE:_length;
		if (lseek(_fd,_length-4,SEEK_SET)==-1)
			THROW_ERROR;

		if (write(_fd,"\0\0\0\0",4)!=4)
			THROW_ERROR;

		_base=mmap(NULL,_length,PROT_WRITE|PROT_READ
				,MAP_SHARED,_fd,_offset);

		if (_base==MAP_FAILED)
			THROW_ERROR;

		if (close(_fd)!=0)		
			THROW_ERROR;

	}

	void _lock(void* addr,size_t length)throw(nynn_error_t)
	{

		if (!check_range(addr,length))
			THROW_ERROR_WITH_ERRNO(ENOMEM);

		if (mlock(addr,length)!=0)
			THROW_ERROR;

		return;
	}

	void _lock()throw(nynn_error_t)
	{
		try{
			_lock(_base,_length);
		}catch (nynn_error_t& err) {
			throw err;
		}

		return; 
	}

	void unlock(void* addr,size_t length)throw(nynn_error_t)
	{

		if (!check_range(addr,length))
			THROW_ERROR_WITH_ERRNO(ENOMEM);

		if (munlock(addr,length)!=0)
			THROW_ERROR;

		return;
	}

	void unlock()throw(nynn_error_t)
	{
		try{
			unlock(_base,_length);
		}catch (nynn_error_t& err) {
			throw err;
		}

		return; 
	}

	void sync(void* addr,size_t length,int flags)throw(nynn_error_t)
	{

		if (!check_range(addr,length))
			THROW_ERROR_WITH_ERRNO(ENOMEM);

		if (msync(addr,length,flags)!=0)
			THROW_ERROR;

		return;
	}

	void sync(int flags)throw(nynn_error_t)
	{
		try{
			sync(_base,_length,flags);
		}catch (nynn_error_t& err){
			throw err;
		}
		return;
	}


	~mmap_file_t()
	{
		try{
			sync(MS_SYNC|MS_INVALIDATE);
		}catch (nynn_error_t& err) {
			cerr<<err.get_error()<<endl;
		}
		munmap(_base,_length);
	}

	void *get_base()const
	{
		return _base;
	}

	size_t get_length()const
	{
		return _length;
	}


private:
		//forbid copying object 
	mmap_file_t(const mmap_file_t&);
	mmap_file_t& operator=(const mmap_file_t&);

	bool check_range(void* &addr,size_t &length)
	{
		size_t pagesize=static_cast<size_t>(sysconf(_SC_PAGESIZE));
		// round down addr by page boundary
		char* taddr=static_cast<char*>(addr);
		char* tbase=static_cast<char*>(_base);

		length += ((size_t)taddr)%pagesize;
		taddr  -= ((size_t)taddr)%pagesize;
		addr=static_cast<void*>(taddr);

		// check [addr,addr+length] in [_base,_base+_length]
		if (taddr-tbase>=0 && taddr-tbase<=_length-length)
			return true;
		else 
			return false;
	}

	string 	_path;
	int 	_fd;
	size_t 	_length;
	off_t 	_offset;
	void*   _base;
};

struct shm_allocator_t{
	void* operator new(size_t size,void*buff){return buff;}
	void  operator delete(void*buff,size_t size){ }
};
struct semid0_t{

	semid0_t(size_t slots)throw(nynn_error_t):_slot_max(slots)
	{	
		_semid=semget(IPC_PRIVATE,_slot_max,IPC_CREAT|IPC_EXCL|0700);
		if (_semid==-1) THROW_ERROR;
		
		uint16_t *array=new uint16_t[_slot_max];
		std::fill(array,array+_slot_max,0);
		if (semctl(_semid,0,SETALL,array)==-1)THROW_ERROR;
	}

	~semid0_t(){}

	size_t get_slot_max()const{return _slot_max;}
	int    get_semid()const {return _semid;}
	
	int    _semid;		
	size_t _slot_max;
};

struct semid1_t{
	semid1_t(int semid):_semid(semid){}
	
	~semid1_t()
	{
		if (semctl(_semid,0,IPC_RMID)==-1)THROW_ERROR;
	}

	int    _semid;
};	

class lockop_t {

public:

	lockop_t(int32_t id)throw(nynn_error_t):_id(id)
	{

		struct seminfo si;
		if (semctl(0,0,IPC_INFO,&si)==-1)THROW_ERROR;
		int semmsl=si.semmsl;
		int semopm=si.semopm;
		log_i("semmsl=%d",semmsl);
		log_i("semopm=%d",semopm);
		
		int semid = _id >> 16;
		int slot  = 0x0000ffff & _id;

		struct semid_ds ds;
		if (semctl(semid,0,IPC_STAT,&ds)!=0)THROW_ERROR;

		if (slot>=ds.sem_nsems)THROW_ERROR_WITH_ERRNO(EINVAL);

		struct sembuf sop;
		sop.sem_op=-1;
		sop.sem_num=slot;
		sop.sem_flg=SEM_UNDO;
		
		if (semop(semid,&sop,1)!=0)THROW_ERROR;
	}

	~lockop_t()throw(nynn_error_t)
	{
		int semid = _id >> 16;
		int slot  = 0x0000ffff & _id;

		struct sembuf sop;
		sop.sem_op=1;
		sop.sem_num=slot;
		
		if (semop(semid,&sop,1)!=0)THROW_ERROR;
	}

private:

	//disallow copy.
	lockop_t(const lockop_t&);
	lockop_t& operator=(const lockop_t&);
	//disallow created on free store(heap);
	void*operator new(size_t);
	void*operator new(size_t,void*);

	uint32_t    _id;
};

struct shmid_t{
	int _shmid;
	size_t _length;
	explicit shmid_t(size_t length)throw(nynn_error_t):
		_shmid(0),_length(length)
	{
		_shmid=shmget(IPC_PRIVATE,_length,IPC_CREAT|IPC_EXCL|0700);
		if (_shmid==-1)
			THROW_ERROR;
	}
	~shmid_t(){}
	int get_shmid()const{return _shmid;}
	size_t get_length()const{return _length;}

private:
	shmid_t(const shmid_t&);
	shmid_t& operator=(const shmid_t&);
};

class shm_t{
public:

	explicit shm_t(int shmid,size_t length=0)throw(nynn_error_t):
		_shmid(shmid),_length(length),_base(0)
	{ 
		_base=shmat(_shmid,NULL,0);
		if(_base==(void*)-1)THROW_ERROR;
	}
	
	explicit shm_t(const shmid_t& id)throw(nynn_error_t):
		_shmid(id._shmid),_length(id._length)
	{
		_base=shmat(_shmid,NULL,0);
		if(_base==(void*)-1)THROW_ERROR;
	}

	~shm_t()
	{
		if(shmdt(_base)==-1)
			THROW_ERROR;

		struct shmid_ds ds;
		if (shmctl(_shmid,IPC_STAT,&ds)==-1)
			THROW_ERROR;

		if (ds.shm_nattch>0)return;
		if (shmctl(_shmid,IPC_RMID,NULL)==-1)
			THROW_ERROR;
	}

	void* get_base()const{return _base;}
	size_t get_length()const{return _length;}
	int   get_shmid()const{return _shmid;}
private:
	shm_t(const shm_t&);
	shm_t& operator=(const shm_t&);

	int    _shmid;
	size_t _length;
	void*  _base;
};

class flock_t{
protected:
	string _path;
	int _fd;

	flock_t(const string& path)throw(nynn_error_t):_path(path){
		if (!file_exist(_path.c_str()))
			THROW_ERROR_WITH_ERRNO(ENOENT);

		_fd=open(_path.c_str(),O_RDWR);
		if (_fd==-1)
			THROW_ERROR_WITH_ERRNO(ENOENT);
	}

public:
	virtual void lock(off_t start,off_t length)=0;
	virtual void unlock(off_t start,off_t length)=0;
	virtual bool trylock(off_t start,off_t length)=0;
	virtual ~flock_t() { close(_fd);}
};

class frlock_t:public flock_t{
public:
	frlock_t(const string& path):flock_t(path){}
	~frlock_t(){}
	virtual void lock(off_t start,off_t length)
	{
		struct flock op;
		op.l_type=F_RDLCK;
		op.l_start=start;
		op.l_len=length;
		op.l_whence=SEEK_SET;
		if (fcntl(_fd,F_SETLKW,&op)!=0)
			THROW_ERROR;
	}
	
	virtual void unlock(off_t start,off_t length)
	{
		struct flock op;
		op.l_type=F_UNLCK;
		op.l_start=start;
		op.l_len=length;
		op.l_whence=SEEK_SET;
		if (fcntl(_fd,F_SETLKW,&op)!=0)
			THROW_ERROR;

	}
	virtual bool trylock(off_t start,off_t length)
	{
		struct flock op;
		op.l_type=F_RDLCK;
		op.l_start=start;
		op.l_len=length;
		op.l_whence=SEEK_SET;
		if (fcntl(_fd,F_SETLK,&op)==0) return true;
		if (errno==EAGAIN) return false;
		THROW_ERROR;
	}
};

class fwlock_t:public flock_t{
public:	
	fwlock_t(const string&path):flock_t(path){}
	~fwlock_t(){} 
	virtual void lock(off_t start,off_t length)
	{
		struct flock op;
		op.l_type=F_WRLCK;
		op.l_start=start;
		op.l_len=length;
		op.l_whence=SEEK_SET;
		if (fcntl(_fd,F_SETLKW,&op)!=0)
			THROW_ERROR;
	}

	virtual void unlock(off_t start,off_t length)
	{
		struct flock op;
		op.l_type=F_UNLCK;
		op.l_start=start;
		op.l_len=length;
		op.l_whence=SEEK_SET;
		if (fcntl(_fd,F_SETLKW,&op)!=0)
			THROW_ERROR;
	}

	virtual bool trylock(off_t start,off_t length)
	{
		struct flock op;
		op.l_type=F_WRLCK;
		op.l_start=start;
		op.l_len=length;
		op.l_whence=SEEK_SET;
		if (fcntl(_fd,F_SETLK,&op)==0) return true;
		if (errno==EAGAIN) return false;
		THROW_ERROR;
	}
};

class raii_flock_t{
public:
	explicit raii_flock_t(flock_t* lck):_lock(lck){_lock->lock(0,0);}
			~raii_flock_t(){_lock->unlock(0,0);}
private:
	flock_t *_lock;
};

string& strerr(int errnum,string& s)
{
	char errbuff[ERR_BUFF_SIZE];
	memset(errbuff,0,ERR_BUFF_SIZE);
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
	//XSI-compliant
	if (strerror_r(ne_errnum,errbuff,ERR_BUFF_SIZE)==0)
		s=errbuff;
	else
		s="Unknown Error!";
#else
	//GNU-specific
	char *msg=strerror_r(errnum,errbuff,ERR_BUFF_SIZE);
	if (msg!=NULL)
		s=msg;
	else
		s="Unknown Error!";
#endif
}

void log_debug(ostream&out,const char*file,const int line,
		const char *function,const int level,int errnum,const char *fmt,...)
{
#ifdef _DEBUG
	va_list ap;
	va_start(ap,fmt);
	vlog(out,file,line,function,level,errnum,fmt,ap);
	va_end(ap);
#endif
}

void log(ostream& out,const char*file,const int line,
		const char *function,const int level,int errnum,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	vlog(out,file,line,function,level,errnum,fmt,ap);
	va_end(ap);

}

void vlog(ostream &out,const char *file,const int line,
		const char *function,const int level,int errnum,const char *fmt,va_list ap)
{
	const char* logs[5]={"INFO","WARN","ERROR","ASSERT","DEBUG"};
	stringstream pack;
	string s;
	char buff[VSNPRINTF_BUFF_SIZE];
	pack<<"####"<<logs[level]<<"@"
		<<file
		<<"#"<<line
		<<"-"<<function<<"():";
	if (errnum!=0){
		pack<<"["<<errnum<<"]"
			<<strerr(errnum,s);
	}else{
		vsnprintf(buff,VSNPRINTF_BUFF_SIZE,fmt,ap);
		buff[VSNPRINTF_BUFF_SIZE-1]='\0';
		pack<<buff;
	}

	out<<pack.str()<<endl;
}

int rand_int()
{
	struct timespec ts;
	ts.tv_sec=0;
	ts.tv_nsec=1;
	clock_nanosleep(CLOCK_MONOTONIC,0,&ts,NULL);

	clock_gettime(CLOCK_MONOTONIC,&ts);
	unsigned seed=static_cast<unsigned>(0x0ffffffffl & ts.tv_nsec);
	return rand_r(&seed);
}

}}}
#endif
