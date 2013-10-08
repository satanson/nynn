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
typedef unsigned long int  uint32_t;
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

	class shm_allocator_t;
	class lock_t;
	class rwlock_t;
	class shm_t;

	class frwlock_t;
	class frlock_t;
	class fwlock_t;
	typedef fwlock_t flock_t;

	class raii_lock_t;
	class raii_rlock_t;
	class raii_wlock_t;
	class raii_frwlock_t;


	string& strerr(int errnum,string& s);
	void log(ostream &out,const char *file,const int line,
			const char *func,const int  level,int errnum,const char *fmt,...);
	int rand_int();
	bool file_exist(const string& path);

#define log_i(msg,...)\
   	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_INFO,0,(msg),##__VA_ARGS__)
#define log_w(msg,...)\
	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_WARN,0,(msg),##__VA_ARGS__)
#define log_e(errnum) \
	log(cerr,__FILE__,__LINE__,__FUNCTION__,LOG_ERROR,errnum,NULL)
#define THROW_ERROR \
	throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__)
#define THROW_ERROR_WITH_ERRNO(errnum) \
	throw nynn_error_t((errnum),__FILE__,__LINE__,__FUNCTION__)

	bool file_exist(const string& path)
	{
		struct stat st;
		switch (stat(path.c_str(),&st)){
		case -1:
			{	
				if(errno==ENOENT)return false;
				else {log_e(errno);return true;}
			}
		default:
			return true;
		}
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
		explicit mmap_file_t(const string& path)
			throw(nynn_error_t)
			:mf_path(path),mf_offset(0),mf_base(0)
			{
				mf_fd=open(path.c_str(),O_RDWR);
				if (mf_fd<0)
					THROW_ERROR;

				mf_length=lseek(mf_fd,0,SEEK_END);
				if (mf_length==-1)
					THROW_ERROR;

				mf_base=mmap(NULL,mf_length,PROT_WRITE|PROT_READ
						,MAP_SHARED,mf_fd,mf_offset);

				if (mf_base==MAP_FAILED)
					THROW_ERROR;

				if (close(mf_fd)!=0)		
					THROW_ERROR;
			}

		//create a shared mapping file for already existed file
		mmap_file_t(const string& path,size_t length,off_t offset)
			throw(nynn_error_t)
			:mf_path(path),mf_offset(offset),mf_base(0)
			{
				mf_fd=open(path.c_str(),O_RDWR);
				if (mf_fd<0)
					THROW_ERROR;

				mf_length=lseek(mf_fd,0,SEEK_END);
				if (mf_length==-1)
					THROW_ERROR;

				mf_length=mf_offset+length<mf_length?length:mf_length-mf_offset;

				mf_base=mmap(NULL,mf_length,PROT_WRITE|PROT_READ
						,MAP_SHARED,mf_fd,mf_offset);
				if (mf_base==MAP_FAILED)
					THROW_ERROR;

				if (close(mf_fd)!=0)		
					THROW_ERROR;

			}

		// create a private mapping file for a new file
		mmap_file_t(const string& path,size_t length)
			throw(nynn_error_t)
			:mf_path(path),mf_length(length),mf_offset(0),mf_base(0)
			{
				mf_fd=open(path.c_str(),O_RDWR|O_CREAT|O_EXCL);
				if (mf_fd<0)
					THROW_ERROR;

				mf_length=lseek(mf_fd,mf_length-4,SEEK_SET);
				if (mf_length==-1)
					THROW_ERROR;

				if (write(mf_fd,"\0\0\0\0",4)!=4)
					THROW_ERROR;

				mf_base=mmap(NULL,mf_length,PROT_WRITE|PROT_READ
						,MAP_SHARED,mf_fd,mf_offset);

				if (mf_base==MAP_FAILED)
					THROW_ERROR;

				if (close(mf_fd)!=0)		
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
				_lock(mf_base,mf_length);
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
				unlock(mf_base,mf_length);
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
				sync(mf_base,mf_length,flags);
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
			munmap(mf_base,mf_length);
		}

		void *get_base()const
		{
			return mf_base;
		}

		size_t get_length()const
		{
			return mf_length;
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
			char* tbase=static_cast<char*>(mf_base);

			length += ((size_t)taddr)%pagesize;
			taddr  -= ((size_t)taddr)%pagesize;
			addr=static_cast<void*>(taddr);

			// check [addr,addr+length] in [mf_base,mf_base+mf_length]
			if (taddr-tbase>=0 && taddr-tbase<=mf_length-length)
				return true;
			else 
				return false;
		}

		string 	mf_path;
		int 	mf_fd;
		size_t 	mf_length;
		off_t 	mf_offset;
		void*   mf_base;
	};
	
	class shm_allocator_t{
	protected:
		void* operator new(size_t size,void*buff){return buff;}
		void  operator delete(void*buff,size_t size){ }
	};

	class lock_t {

	public:

		void lock()throw(nynn_error_t)
		{
			struct sembuf sop;
			sop.sem_op=-1;
			sop.sem_num=0;
			sop.sem_flg=SEM_UNDO;
			
			if (semop(_semid,&sop,1)==0)return;

			THROW_ERROR;
		}

		bool trylock()throw(nynn_error_t)
		{
			struct sembuf sop;
			sop.sem_op=-1;
			sop.sem_num=0;
			sop.sem_flg=SEM_UNDO|IPC_NOWAIT;
			
			if (semop(_semid,&sop,1)==0)return true;
			if (errno==EAGAIN)return false;

			THROW_ERROR;
		}

		void unlock()throw(nynn_error_t)
		{
			struct sembuf sop;
			sop.sem_op=1;
			sop.sem_num=0;
			
			if (semop(_semid,&sop,1)==0)return;

			THROW_ERROR;
		}

		~lock_t()throw(nynn_error_t)
		{
			_tag=INVALID_TAG;
			if (semctl(_semid,0,IPC_RMID)==0)return;
			THROW_ERROR;
		}

		bool check_valid()
		{
			return (_tag==LOCK_TAG && check_exist(_semid));
		}
		
		static lock_t* get_lock(void*shm,size_t size)throw(nynn_error_t)
		{
			assert(size>=sizeof(lock_t));
			lock_t *lck=static_cast<lock_t*>(shm); 

			if(lck->_tag==LOCK_TAG && check_exist(lck->_semid))return lck;
			else return new(shm)lock_t();
		}

	private:

		friend class rwlock_t;

		lock_t(const lock_t&);
		lock_t& operator=(const lock_t&);
		
		lock_t(const string& path,int id)throw(nynn_error_t):
			_path(path),_id(id)
		{
			flock_t lck(_path);
		   	raii_frwlock_t get(&lck);	

			struct seminfo synctl;
			if (semctl(0,0,IPC_INFO,&sysctl)==-1)THROW_ERROR;
			_semmsl=synctl.semmsl;
			_semopm=synctl.semopm;
			log_i("semmsl=%d",_semmsl);
			log_i("semopm=%d",_semopm);

			proj_id = id%0xff+1;
			slot_id = id%(_semmsl-1)+1;
			_semid=semget(ftok(_path.c_str(),proj_id),_semmsl,IPC_CREAT|0770);
			if (_semid==-1)THROW_ERROR;

			log_i("create a new _lock");

			if (semctl(_semid,0,SETVAL,1)!=0)
				THROW_ERROR;
		}

		static bool check_exist(int semid)
		{
			struct semid_ds ds;
			if (semctl(semid,0,IPC_STAT,&ds)==0) return true;
			if (errno==EINVAL)return false;
			return true;
		}

		static bool check_nonexist(int semid)
		{
			return !check_exist(semid);
		}

		string _path;
		int    _id;
		int    _semid;
		int    _semmsl;
		int    _semopm;
	};	

	class raii_lock_t{
	public:
		explicit raii_lock_t(lock_t*lck):_lock(lck){_lock->lock();}
				~raii_lock_t(){_lock->unlock();}
	private:
		lock_t *_lock;
	};

	class rwlock_t:shm_allocator_t{
	
	public:

		void rlock()
		{
			raii_lock_t get(&_mutex);

			if (_rnum!=0){
				_rnum++;
			}else{
				_lock.lock();
				assert(_rnum==0);
				_rnum++;
			}
		}

		bool tryrlock()
		{
			raii_lock_t get(&_mutex);
			if (_rnum!=0) { 
				_rnum++;
				return true;
			}else if (_lock.trylock()) {
				_rnum++;
				return true;
			}else {
				return false;
			}
		
		}
		

		void unrlock()
		{
			raii_lock_t get(&_mutex);

			if (_rnum>1){
				_rnum--;
				return;
			}else {
				_lock.unlock();
				_rnum--;
			}
		}

		void wlock() { _lock.lock();}

		int  trywlock() { return _lock.trylock();}
		
		void unwlock() { _lock.unlock();}
		
		~rwlock_t() { _tag=RWLOCK_TAG; }

		bool check_valid()
		{
			return _tag==RWLOCK_TAG && 
				   _mutex.check_valid() &&
				   _lock.check_valid();
		}

		static rwlock_t* get_rwlock(void*shm,size_t size)
		{
			assert(size>=sizeof (rwlock_t));

			rwlock_t* rwlck=static_cast<rwlock_t*>(shm);
			if (rwlck->check_valid())return rwlck;
			else return new(shm)rwlock_t();

		}
	private:
		rwlock_t(const rwlock_t&);
		rwlock_t& operator=(const rwlock_t&);
		rwlock_t( )throw (nynn_error_t):_tag(RWLOCK_TAG),_rnum(0){}

		int  		_tag;
		lock_t 		_mutex;
		mutable int _rnum;
		lock_t 		_lock;
	};

	class shm_t:public shm_allocator_t{
		public:
			void* get_base() { return shmat(_shmid,NULL,0); }

			~shm_t()
			{
				struct shmid_ds ds;
				if (shmctl(_shmid,IPC_STAT,&ds)==-1)
					THROW_ERROR;

				if (ds.shm_nattch>0)return;
				if (shmctl(_shmid,IPC_RMID,NULL)!=0)
					THROW_ERROR;
			}

			static void detach(void* base)
			{
				if(shmdt(base)==-1)
					THROW_ERROR;
			}

			static const shm_t* get_shm(void *shm,size_t size,size_t length)
			{
				assert(size>=sizeof(shm_t));
				shm_t* shmptr=static_cast<shm_t*>(shm);

				if (shmptr->check_valid())return shmptr;
				
				return new(shm)shm_t(length);
			}

		private:
			shm_t(const shm_t&);
			shm_t& operator=(const shm_t&);

			explicit shm_t(size_t length)throw(nynn_error_t)
				:_tag(SHM_TAG),_length(length)
			{
				_shmid=shmget(IPC_PRIVATE,_length,IPC_CREAT|IPC_EXCL|0700);
				if (_shmid==-1)
					THROW_ERROR;

			}

			bool check_valid()
			{
				if (_tag!=SHM_TAG)return false;

				struct shmid_ds ds;
				switch(shmctl(_shmid,IPC_STAT,&ds)){
				case 0:return true;
				case -1:
					{
						if(errno==EINVAL || errno==EIDRM)return false;
						return true;
					}
				default:{log_e(errno);return true;}
				}
			}

			int _tag;
			int _shmid;
			int _length;

	};

	class frwlock_t{
	protected:
		string _path;
		int _fd;

		frwlock_t(const string& path)throw(nynn_error_t):_path(path){
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
		virtual ~frwlock_t() { close(_fd);}
	};
	
	class frlock_t:public frwlock_t{
	public:
		frlock_t(const string& path):frwlock_t(path){}
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
	
	class fwlock_t:public frwlock_t{
	public:	
		fwlock_t(const string&path):frwlock_t(path){}
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

	class raii_rlock_t{
	public:
		explicit raii_rlock_t(rwlock_t* lck):_lock(lck){_lock->rlock();}
				~raii_rlock_t(){_lock->unrlock();}
	private:
		rwlock_t *_lock;
	};

	class raii_wlock_t{
	public:
		explicit raii_wlock_t(rwlock_t* lck):_lock(lck){_lock->wlock();}
				~raii_wlock_t(){_lock->unwlock();}
	private:
		rwlock_t *_lock;
	};

	class raii_frwlock_t{
	public:
		explicit raii_frwlock_t(frwlock_t* lck):_lock(lck){_lock->lock(0,0);}
				~raii_frwlock_t(){_lock->unlock(0,0);}
	private:
		frwlock_t *_lock;
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


	void log(ostream &out,const char *file,const int line,
			const char *function,const int  level,int errnum,const char *fmt,...)
	{
		const char* logs[3]={"INFO","WARN","ERROR"};
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
			va_list ap;
			va_start(ap,fmt);
			vsnprintf(buff,VSNPRINTF_BUFF_SIZE,fmt,ap);
			va_end(ap);
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
