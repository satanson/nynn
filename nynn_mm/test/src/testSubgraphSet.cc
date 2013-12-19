#include<test.h>

int main(int argc,char**argv)
{
	string basedir=argv[1];
	SubgraphSet sgs(argv[1]);
	vector<int32_t> keys;
	sgs.getSubgraphKeys(keys);
	for (int i=0;i<keys.size();i++){
		cout<<"0x"<<hex<<setw(8)<<setfill('0')<<keys[i]<<endl;
	}
}
