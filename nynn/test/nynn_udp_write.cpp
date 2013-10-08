#include<nynn.h>
int main(int argc,char*argv[])
{
	size_t wsize=4096;
	char *host;
	char *msgid;
	char *ownmsgid;
	while(true){
		int ch=getopt(argc,argv,":+s:H:i:I:h");
		if(ch=='?'){
			cout<<"nynn_udp_write:invalid option -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_udp_write [-s num] [-H host] [-i msgid] [-I msgid]"<<endl;
			cout<<"Try \"nynn_udp_write -h for more help\""<<endl;
			exit(0);
		}else if (ch==':'){
			cout<<"nynn_udp_write:option requires argument -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_udp_write [-s num] [-H host] [-i msgid] [-I msgid]"<<endl;
			cout<<"Try \"nynn_udp_write -h for more help\""<<endl;
			exit(0);
		}else if (ch==-1){
			break;
		}else if (ch=='s'){
			wsize=(atoi(optarg)<=0?wsize:atoi(optarg));
		}else if (ch=='H'){
			host=optarg;
		}else if (ch=='i'){
			msgid=optarg;
		}else if (ch=='I'){
			ownmsgid=optarg;
		}else {
			cout<<"Usage:nynn_udp_write [-s num] [-H host] [-i msgid] [-I msgid]"<<endl;
			cout<<"nynn_udp_write is a client for writing message."<<endl;
			cout<<"Options:"<<endl;
			cout<<"\t-s the wsize in byte of buff for writting(default 4096)"<<endl;
			cout<<"\t-H the target host receiving msg"<<endl;
			cout<<"\t-i the identifier of msg that is sent to target host"<<endl;
			cout<<"\t-I the identifier of msg that want to receive"<<endl;
			cout<<"\t-h help nynn_udp_write"<<endl;
			exit(0);
		}
	}
	
	cout<<"msgid="<<msgid<<endl;
	cout<<"ownmsgid="<<ownmsgid<<endl;
	cout<<"host="<<host<<endl;
	nynn_tap_t tap(ownmsgid);
	char *wbuff=new char[wsize];
	while(true){
		memset(wbuff,0,wsize);
		if(cin.eof())break;
		cin.getline(wbuff,wsize);
		tap.infuse(host,msgid,wbuff,wsize);
	}
	return 0;
}
