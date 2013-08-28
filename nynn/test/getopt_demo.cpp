#include<unistd.h>
#include<iostream>
#include<cstdlib>
using namespace std;

int main(int argc,char*argv[])
{
#if 1
	char optstring[64]={0};
	cin.getline(optstring,64);
	while(true){
		int ch=0;
		ch=getopt(argc,argv,optstring);
		cout<<"-----------------------"<<endl;
		cout<<"optind="<<optind<<endl;
		cout<<"optopt="<<optopt<<endl;
		cout<<"opterr="<<opterr<<endl;
		if(optarg)cout<<"optarg="<<optarg<<endl;
		cout<<"ch="<<ch<<endl;
		cout<<"-----------------------"<<endl;
		switch(ch){
			case '?':
				cout<<"unknown option!"<<endl;
				break;
			case ':':
				cout<<"missing option argument!"<<endl;
				break;
			case -1:
				exit(0);
			default:
				break;
		}
	}
#endif
	return 0;
}
