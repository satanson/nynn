#include<nynn.h>
int main(int argc,char*argv[])
{
	uint16_t port=30001;
	int hose=0;

	while(true){
		int ch=getopt(argc,argv,":+p:H:h");
		if(ch=='?'){
			cout<<"nynn_cli_read:invalid option -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_cli_read [-p port] [-H hose] "
			    <<"addr1 addr2... addrn "<<endl;

			cout<<"Try \"nynn_cli_read -h for more help\""<<endl;
			exit(0);
		}else if (ch==':'){
			cout<<"nynn_cli_read:option requires argument -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_cli_read [-p port] [-H hose] "
			    <<"addr1 addr2... addrn "<<endl;
			cout<<"Try \"nynn_cli_read -h for more help\""<<endl;
			exit(0);
		}else if (ch==-1){
			break;
		}else if (ch=='p'){
			port=(atoi(optarg)<=0?port:atoi(optarg));
		}else if (ch=='H'){
			hose=(atoi(optarg)<=0?hose:atoi(optarg));
		}else{ 
			cout<<"Usage:nynn_cli_read [-p port] [-H hose] "
			    <<"addr1 addr2... addrn "<<endl;
			cout<<"nynn_cli_read is a client for reading message."<<endl;
			cout<<"Options:"<<endl;
			cout<<"\t-p the local port of nynn_daemon(default 30001)"<<endl; 
			cout<<"\t-H the hoseno of tap(default 0)"<<endl;
			cout<<"\t-h help nynn_cli_read"<<endl;
			exit(0);
		}
	}

	char *rbuff;
	size_t rbuffsize;
	nynn_tap_t tap(port,hose);
	while(true){
		if(tap.read(&rbuff,&rbuffsize)!=0){
			cout<<"failed to read!"<<endl;
			continue;
		}
		cout<<rbuff<<endl;
		delete rbuff;
	}
	return 0;
}
