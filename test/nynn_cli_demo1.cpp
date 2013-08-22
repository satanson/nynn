#include<nynn.h>
int main()
{
	char *shm;
	int shmid=nynn_shmat(-1,(void**)&shm,4096,false);
	cin.getline(shm,4096);
	nynn_write(shmid,4096);
	return 0;
}
