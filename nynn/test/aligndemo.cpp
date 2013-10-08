#include<iostream>
using namespace std;
struct nynn_msg_t{
	struct {
	size_t msgsize;
    char   host[16];
	char   msgid[40];
	}msghdr;
	char   msgbody[1];
};
int main()
{
	nynn_msg_t msg;
	cout<<"sizeof(msg)"<<sizeof(msg)<<endl;
	cout<<"sizeof(msghdr)"<<sizeof(msg.msghdr)<<endl;
	cout<<"sizeof(msgbody)"<<sizeof(msg.msgbody)<<endl;
	return 0;
}
