#include "public.h"

void __log(
		ostream &out,
		const char*file,
		const int line,
		const char *func,
		const logtype_t type,
		const int errnum,
		const char*fmt,
		...
		)
{
#ifdef DEBUG
	const char* strlogs[3]={"INFO","WARN","ERROR"};
	char msg[256];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(msg,256,fmt,ap);
	va_end(ap);
	char buff[128]={0};
	char *err=strerror_r(errnum,buff,sizeof(buff));
	out<<strlogs[type]
		<<"[file=>'"<<file
		<<"', line=>'"<<line
		<<"', function=>'"<<func
		<<"', errno=>'"<<errnum
		<<"', error=>'"<<err
		<<"', msg=>'"<<msg<<"']"<<endl;
	out.flush();
#endif
}
void __exit_on_error(
		ostream &out,
		const char*file,
		const int line,
		const char *func,
		const int errnum,
		const char*msg,
		bool thread   
		)
{
	char buff[128]={0};
	char *err=strerror_r(errnum,buff,sizeof(buff));
	out<<"ERROR[file=>'"<<file
		<<"', line=>'"<<line
		<<"', function=>'"<<func
		<<"', errno=>'"<<errnum
		<<"', error=>'"<<err
		<<"', msg=>'"<<msg<<"']"<<endl;
	out.flush();

	if (thread){
		pthread_exit(NULL);
	}else{
		exit(errnum);
	}
}

void setNonBlocking(int sockfd)
{
	int flags;
	if ((flags=fcntl(sockfd,F_GETFL))==-1){
		exit_on_error(errno,"failed to get flags of sockfd ");
	}
	flags|=O_NONBLOCK;
	if (fcntl(sockfd,F_SETFL,flags)==-1){
		exit_on_error(errno,"failed to set flags of sockfd ");
	}
}

void initSockaddr(struct sockaddr_in *addr_ptr,const char*host,short port)
{
	bzero(addr_ptr,sizeof(*addr_ptr));
	addr_ptr->sin_family = AF_INET;
	if (inet_pton(AF_INET,host,&addr_ptr->sin_addr)!=1){
		exit_on_error(errno,"failed to convert a string to AF_INET address");
	}
	addr_ptr->sin_port = htons(port);

}
uint32_t gethostaddr(char *hostname,size_t size)
{
	uint32_t addr;
	char straddr[INET_ADDRSTRLEN];
	if (-1==gethostname(hostname,size)){
		exit_on_error(errno,"failed to get hostname!");
	}

	struct addrinfo  hint, *res, *p;
	memset(&hint,0,sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_flags = AI_ADDRCONFIG;
	
	if (-1==getaddrinfo(hostname,NULL,&hint,&res)){
		exit_on_error(errno,"failed to get host inet4 addr");
	}
	if (res!=NULL){
		p=res;
		do{
			addr=((sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
			inet_ntop(AF_INET,&addr,straddr,INET_ADDRSTRLEN);
			if (strcmp(straddr,"127.0.0.1")!=0){
				return addr;
			}
			p=p->ai_next;
		}while(NULL!=p);
		freeaddrinfo(res);

	}else{
		return 0;
	}

}

char* ltrim(const char *chars,const char *src, char *dest)
{
	string s(src);
	if (strlen(src)==0){
		dest[0]='\0';
	}else if (s.find_first_not_of(chars)==string::npos){
		dest[0]='\0';
	}else{
		string s1=s.substr(s.find_first_not_of(chars));
		strcpy(dest,s1.c_str());
	}
	return dest;
}

char* rtrim(const char *chars,const char *src, char *dest)
{
	string s(src);
	if (strlen(src)==0){
		dest[0]='\0';
	}else if(s.find_last_not_of(chars)==string::npos){
		dest[0]='\0';
	}else{		
		string s1=s.substr(0,s.find_last_not_of(chars)+1);
		strcpy(dest,s1.c_str());
	}
	return dest;
}

char* chop(const char ch,const char *src,char *dest)
{	
	if (strlen(src)==0){
		dest[0]='\0';
	}else{
		string s(src);
		string s1=s.substr(0,s.find_first_of(ch));
		strcpy(dest,s1.c_str());
	}
	return dest;
}
