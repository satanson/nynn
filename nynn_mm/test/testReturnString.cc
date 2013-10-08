#include<iostream>
#include<string>
using namespace std;
const char* get_string()
{
	string s("my name is ranpanf");
	s.append("\nhi ranpanf!");
	return s.c_str();
}

int main()
{
	cout<<get_string()<<endl;
}
