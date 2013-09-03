#include<nynn.h>
int main(int argc,char*argv[])
{
	size_t wsize=4096;
	uint16_t port=30001;
	int hose=0;
	int n=2;

	while(true){
		int ch=getopt(argc,argv,":+s:p:H:n:h");
		if(ch=='?'){
			cout<<"nynn_cli_write:invalid option -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_cli_write [-s num] [-p port] [-H hose] [-n num] "
			    <<"addr1 addr2... addrn "<<endl;

			cout<<"Try \"nynn_cli_write -h for more help\""<<endl;
			exit(0);
		}else if (ch==':'){
			cout<<"nynn_cli_write:option requires argument -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_cli_write [-s num] [-p port] [-H hose] [-n num] "
			    <<"addr1 addr2... addrn "<<endl;
			cout<<"Try \"nynn_cli_write -h for more help\""<<endl;
			exit(0);
		}else if (ch==-1){
			break;
		}else if (ch=='s'){
			wsize=(atoi(optarg)<=0?wsize:atoi(optarg));
		}else if (ch=='p'){
			port=(atoi(optarg)<=0?port:atoi(optarg));
		}else if (ch=='H'){
			hose=(atoi(optarg)<=0?hose:atoi(optarg));
		}else if (ch=='n'){
			n=(atoi(optarg)<=0?n:atoi(optarg));
		}else {
			cout<<"Usage:nynn_cli_write [-s num] [-p port] [-H hose] [-n num] "
			    <<"addr1 addr2... addrn "<<endl;
			cout<<"nynn_cli_write is a client for writing message."<<endl;
			cout<<"Options:"<<endl;
			cout<<"\t-s the wsize in byte of buff for writting(default 4096)"<<endl;
			cout<<"\t-p the local port of nynn_daemon(default 30001)"<<endl; 
			cout<<"\t-H the hoseno of tap(default 0)"<<endl;
			cout<<"\t-n the number of targets receiving data(default 2)"<<endl;
			cout<<"\t-h help nynn_cli_write"<<endl;
			exit(0);
		}
	}
	
	uint32_t *inetaddr=new uint32_t[n];
	size_t i=0;
	for (i=0;i<n;i++){
		inet_pton(AF_INET,argv[optind+i],&inetaddr[i]);
	}

	nynn_tap_t tap(port,hose);
	char *wbuff=new char[wsize];
	while(true){
		memset(wbuff,0,wsize);
		if(cin.eof())break;
		cin.getline(wbuff,wsize);
		tap.write(inetaddr,n,wbuff,wsize);
	}
	return 0;
}
