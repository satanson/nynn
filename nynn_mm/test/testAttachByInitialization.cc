#include<iostream>
using namespace std;

namespace nynn{namespace mm{
	class Resource{
		public:
			Resource(){
				cout<<"Initialize Resource\n";
			}

			~Resource(){
				cout<<"Release Resource\n";
			}

	}global_res;
}}

int main(){
	cout<<"main\n";
}
