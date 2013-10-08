#include<iostream>
#include<sys/ipc.h>
#include<sys/shm.h>

using namespace std;

int main()
{
	struct shminfo info;
	shmctl(0,IPC_INFO,(struct shmid_ds*)&info);
	cout<<info.shmmax<<endl;
	return 0;
}
