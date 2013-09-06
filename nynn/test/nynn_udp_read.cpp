#include<nynn.h>
int main(int argc,char*argv[])
{
	char *ownmsgid;
	int n=10;
	if (argc==1){
		cout<<"Usage:nynn_udp_read [-I msgid] [-n num]"<<endl;
		cout<<"nynn_udp_read is a client for writing message."<<endl;
		cout<<"Options:"<<endl;
		cout<<"\t-I the identifier of msg that want to receive "<<endl;
		cout<<"\t-n the number of msg will be received!"<<endl;
		cout<<"\t-h help nynn_udp_read"<<endl;
		exit(0);
	}
	while(true){
		int ch=getopt(argc,argv,":+I:n:h");
		if(ch=='?'){
			cout<<"nynn_udp_read:invalid option -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_udp_read [-I msgid] [-n num]"<<endl;
			cout<<"Try \"nynn_udp_read -h for more help\""<<endl;
			exit(0);
		}else if (ch==':'){
			cout<<"nynn_udp_read:option requires argument -- '"
				<<(char)optopt<<"'"<<endl;
			cout<<"Usage:nynn_udp_read [-I msgid] [-n num]"<<endl;
			cout<<"Try \"nynn_udp_read -h for more help\""<<endl;
			exit(0);
		}else if (ch==-1){
			break;
		}else if (ch=='n'){
			n=(atoi(optarg)<0?n:atoi(optarg));
		}else if (ch=='I'){
			ownmsgid=optarg;
		}else {
			cout<<"Usage:nynn_udp_read [-I msgid] [-n num]"<<endl;
			cout<<"nynn_udp_read is a client for writing message."<<endl;
			cout<<"Options:"<<endl;
			cout<<"\t-I the identifier of msg that want to receive "<<endl;
			cout<<"\t-n the number of msg will be received!"<<endl;
			cout<<"\t-h help nynn_udp_read"<<endl;
			exit(0);
		}
	}
	cout<<"ownmsgid="<<ownmsgid<<endl;
	nynn_tap_t tap(ownmsgid);
	char *msgbdy;
	size_t msgbdysize;
	for (size_t i=0;i<n;i++){
		if (tap.effuse(&msgbdy,&msgbdysize)!=0){
			cout<<"failed to effuse msg from tap"<<endl;
			exit(0);
		}
		delete msgbdy;
		cout<<msgbdy<<endl;
	}
	return 0;
}
