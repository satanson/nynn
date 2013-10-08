#ifndef BOSS_H_BY_SATANSON
#define BOSS_H_BY_SATANSON

#include <pthread.h>
#include <assignment.h>
#include <list>
using namespace std;

class Boss;
class Employee;

class Boss{
	private:
		list<Assignment*> _assignments;
		list<Employee*> _employees;
		pthread_mutex_t _lock;
		pthread_cond_t _notEmpty;
		pthread_cond_t _notFull;

		int _maxEmployeeNum;
		int _maxAssignmentNum;
		
	public:
		Boss(int maxEmployeeNum,int maxAssignmentNum);
		~Boss();

		void putAssignment(Assignment* as);
		Assignment* pickAssignment();

		int getAssignmentNum()const;
		void assign(Assignment* as);
		void seal();
		int cease();
		int trycease();
		int timedcease(int msec);
};
class Employee{
	private:
		pthread_t _tid;
	public:
		pthread_t* getID();
		Employee();
		~Employee();
		void undertake(Boss*boss);
		static void* work(void*);
};
#endif

