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

namespace nynn{ namespace mm{ namespace common
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
					,const char*function,const int line)
		:ne_errnum(errnum),ne_file(file),ne_function(function),ne_line(line)
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
			pack<<"ERROR@ "
				<<ne_file
				<<"#"<<ne_line
				<<"--"<<ne_function<<":"
				<<"("<<ne_errnum<<")"
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
		const char* ne_function;
		const int 	ne_line;
		string 		ne_msg;
		void* 		ne_frames[BACKTRACE_FRAME_SIZE];
		size_t 		ne_framesize;
	};

	class mmap_file_t{
	public:
		explicit mmap_file_t(const string& path)
		:mf_path(path),mf_base(0)
		{
			mf_fd=open(path.c_str(),O_RDWR);
		}
		
		mmap_file_t(const string& path,size_t length,off_t offset)
		:mf_path(path),mf_length(length),mf_offset(length),mf_base(0)
		{

		}

		~mmap_file_t();
		void *get_base_addr()const;

		static ssize_t get_file_length(const string& path)
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
