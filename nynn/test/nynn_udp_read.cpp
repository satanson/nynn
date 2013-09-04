#include<nynn.h>
int main(int argc,char*argv[])
{
	char *ownmsgid;
	if (argc==1){
		cout<<"Usage:nynn_udp_read [-I msgid]"<<endl;
		cout<<"nynn_udp_read is a client for writing message."<<endl;
		cout<<"Options:"<<endl;
		cout<<"\t-I the identifier of msg that want to receive "<<endl;
		cout<<"\t-h help nynn_udp_read"<<endl;
		exit(0);
	}
	while(true){
		int ch=getopt(argc,argv,":+I:h");
		if(ch=='?'){
			cout<<"nynn_udp_read:invalid option -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_udp_read [-I msgid]"<<endl;
			cout<<"Try \"nynn_udp_read -h for more help\""<<endl;
			exit(0);
		}else if (ch==':'){
			cout<<"nynn_udp_read:option requires argument -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_udp_read [-I msgid]"<<endl;
			cout<<"Try \"nynn_udp_read -h for more help\""<<endl;
			exit(0);
		}else if (ch==-1){
			break;
		}else if (ch=='I'){
			ownmsgid=optarg;
		}else {
			cout<<"Usage:nynn_udp_read [-I msgid]"<<endl;
			cout<<"nynn_udp_read is a client for writing message."<<endl;
			cout<<"Options:"<<endl;
			cout<<"\t-I the identifier of msg that want to receive "<<endl;
			cout<<"\t-h help nynn_udp_read"<<endl;
			exit(0);
		}
	}
	
	nynn_tap_t tap(ownmsgid);
	char *msgbdy;
	size_t msgbdysize;
	while(true){
		tap.effuse(&msgbdy,&msgbdysize);
		cout<<msgbdy<<endl;
	}
	return 0;
}
