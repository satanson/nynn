#ifndef NYNN_MM_COMMON_BY_SATANSON
#define NYNN_MM_COMMON_BY_SATANSON

#include<iostream>
#include<sstream>
#include<string>
#include<cstring>
#include<cstdlib>
#include<cerrno>

#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<execinfo.h>
using namespace std;

namespace nynn{ namespace mm{ namespace common{
	enum{
		ERR_BUFF_SIZE=128,
		ERR_MSG_RESERVED_SIZE=256,
		BACKTRACE_FRAME_SIZE=20
	};
	class nynn_error_t;
	class mmap_file_t;
	
	class nynn_error_t{
	public:
		nynn_error_t(const int errnum,const char* file
					,const int line,const char*function)
		:ne_errnum(errnum),ne_file(file),ne_line(line),ne_function(function)
		{
			char errbuff[ERR_BUFF_SIZE];
			memset(errbuff,0,ERR_BUFF_SIZE);
			string strerr;
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
			//XSI-compliant
			if (strerror_r(ne_errnum,errbuff,ERR_BUFF_SIZE)==0)
				strerr=errbuff;
			else
				strerr="Unknown Error!";
#else
			//GNU-specific
			char *msg=strerror_r(ne_errnum,errbuff,ERR_BUFF_SIZE);
			if (msg!=NULL)
				strerr=msg;
			else
				strerr="Unknown Error!";
#endif
			stringstream pack;
			pack<<"ERROR@"
				<<ne_file
				<<"#"<<ne_line
				<<"-"<<ne_function<<"():"
				<<"["<<ne_errnum<<"]"
				<<strerr;
			
			ne_msg.reserve(ERR_MSG_RESERVED_SIZE);
			ne_msg+=pack.str();
			ne_framesize=backtrace(ne_frames,BACKTRACE_FRAME_SIZE);

		}
		const char* geterrormsg()const
		{
			return ne_msg.c_str();
		}
		void printbacktrace(){
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
		explicit mmap_file_t(const string& path)
			throw(nynn_error_t)
			:mf_path(path),mf_offset(0),mf_base(0)
		{
			mf_fd=open(path.c_str(),O_RDWR);
			if (mf_fd<0)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);

			mf_length=lseek(mf_fd,0,SEEK_END);
			if (mf_length==-1)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);
			
			mf_base=mmap(NULL,mf_length,PROT_WRITE|PROT_READ
					,MAP_SHARED,mf_fd,mf_offset);

			if (mf_base==MAP_FAILED)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);

			if (close(mf_fd)!=0)		
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);
		}
		
		mmap_file_t(const string& path,size_t length,off_t offset)
			throw(nynn_error_t)
			:mf_path(path),mf_offset(offset),mf_base(0)
		{
			mf_fd=open(path.c_str(),O_RDWR);
			if (mf_fd<0)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);

			mf_length=lseek(mf_fd,0,SEEK_END);
			if (mf_length==-1)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);

			mf_length=mf_offset+length<mf_length?length:mf_length-mf_offset;

			mf_base=mmap(NULL,mf_length,PROT_WRITE|PROT_READ
					,MAP_SHARED,mf_fd,mf_offset);
			if (mf_base==MAP_FAILED)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);

			if (close(mf_fd)!=0)		
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);
			
		}
		mmap_file_t(const string& path,size_t length)
			throw(nynn_error_t)
			:mf_path(path),mf_length(length),mf_offset(0),mf_base(0)
		{
			mf_fd=open(path.c_str(),O_RDWR|O_CREAT|O_EXCL);
			if (mf_fd<0)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);

			mf_length=lseek(mf_fd,mf_length,SEEK_SET);
			if (mf_length==-1)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);
			
			mf_base=mmap(NULL,mf_length,PROT_WRITE|PROT_READ
					,MAP_SHARED,mf_fd,mf_offset);

			if (mf_base==MAP_FAILED)
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);

			if (close(mf_fd)!=0)		
				throw nynn_error_t(errno,__FILE__,__LINE__,__FUNCTION__);

		}

		~mmap_file_t()
		{
			munmap(mf_base,mf_length);
		}
		
		void *getbaseaddr()const
		{
			return mf_base;
		}

		size_t getlength()const
		{
			return mf_length;
		}

		static ssize_t getfilelength(const string& path)
		{
			struct stat st;
			if (stat(path.c_str(),&st)!=0){
				return -1;
			}else{
				return st.st_size;
			}
		}

	private:
		//forbid copying object 
		mmap_file_t(const mmap_file_t&);
		mmap_file_t& operator=(const mmap_file_t&);

		string 	mf_path;
		int 	mf_fd;
		size_t 	mf_length;
		off_t 	mf_offset;
		void*   mf_base;
	};

}}}

#endif
