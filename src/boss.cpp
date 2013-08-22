#include "boss.h"
#include "public.h"
/*----------------------class Boss-------------------*/
Boss::Boss(int maxEmployeeNum,int maxAssignmentNum)
{
	this->_maxEmployeeNum = maxEmployeeNum;
	this->_maxAssignmentNum = maxAssignmentNum;
	this->_lock = PTHREAD_MUTEX_INITIALIZER;
	this->_notFull = PTHREAD_COND_INITIALIZER;
	this->_notEmpty = PTHREAD_COND_INITIALIZER;
	for (int i=0;i<maxEmployeeNum;i++){
		Employee *tmpe=new Employee;
		this->_employees.push_back(tmpe);
		tmpe->undertake(this);
	}
}
Boss::~Boss(){
	pthread_mutex_destroy(&this->_lock);
	pthread_cond_destroy(&this->_notFull);
	pthread_cond_destroy(&this->_notEmpty);

	while (!this->_assignments.empty()){
		Assignment *as=this->_assignments.front();
		delete as;
		this->_assignments.pop_front();
	}
	
	while (!this->_employees.empty()){
		Employee *e = this->_employees.front();
		delete e;
		this->_employees.pop_front();
	}
}
int  Boss:: getAssignmentNum()const{
	return this->_assignments.size();
}

void Boss::putAssignment(Assignment*as)
{
	pthread_mutex_lock(&this->_lock);

	while (getAssignmentNum() == this->_maxAssignmentNum)
		pthread_cond_wait(&this->_notFull,&this->_lock);

	this->_assignments.push_back(as);

	pthread_mutex_unlock(&this->_lock);

	pthread_cond_broadcast(&this->_notEmpty);
}

Assignment* Boss::pickAssignment()
{
	Assignment *as=NULL;

	pthread_mutex_lock(&this->_lock);

	while (getAssignmentNum() == 0)
		pthread_cond_wait(&this->_notEmpty,&this->_lock);

	as=this->_assignments.front();
	this->_assignments.pop_front();

	pthread_mutex_unlock(&this->_lock);

	pthread_cond_broadcast(&this->_notFull);

	return as;
}

void Boss::assign(Assignment* as)
{
	putAssignment(as);	
}

int Boss::cease(){
	list<Employee*>::iterator it=this->_employees.begin();
	while(it!=this->_employees.end()){
		cout<<"cancel a thread"<<endl;
		pthread_t *tid = (*it)->getID();

		pthread_cancel(*tid);
		pthread_join(*tid,NULL);
		it++;
	}
	return 0;
}
int  Boss::trycease(){
	int errnum=0,ret=0;
	list<Employee*>::iterator it=this->_employees.begin();
	while(it!=this->_employees.end()){
		pthread_t *tid = (*it)->getID();
		pthread_cancel(*tid);
		if ((errnum=pthread_tryjoin_np(*tid,NULL))!=0){

			ret=errnum;
		}
		it++;
	}
	return ret;

}

struct sync_args_t{
	pthread_mutex_t triggerLock;
	pthread_mutex_t timeoutLock;
	pthread_cond_t timeout;
	void* val;
};

void static _int_handler(int sig){
	pthread_exit((void*)EINTR);
}

void* _func(void*arg){

	struct sigaction sa_int;
	sa_int.sa_handler=_int_handler;
	if (sigaction(SIGINT,&sa_int,NULL)==-1){
		thread_exit_on_error(errno,"failed to register SIGINT handler");
	}

	struct sync_args_t* sync_ptr = (struct sync_args_t*)arg;
	Boss *boss = (Boss*)sync_ptr->val;


	pthread_mutex_lock(&sync_ptr->triggerLock);
	boss->cease();
	
	pthread_cond_broadcast(&sync_ptr->timeout);
	
	return  (void*)0;
}

int _timedcease(Boss*boss,int msec)
{

	struct sync_args_t sync;
	sync.triggerLock =	PTHREAD_MUTEX_INITIALIZER;
	sync.timeoutLock =  PTHREAD_MUTEX_INITIALIZER;

	sync.val=(void*)boss;


	pthread_condattr_t condattr;
	pthread_condattr_init(&condattr);
	pthread_condattr_setclock(&condattr,CLOCK_MONOTONIC);
	pthread_cond_init(&sync.timeout,&condattr);


	pthread_t tid;
	int errnum;

	pthread_mutex_lock(&sync.triggerLock);
	pthread_mutex_lock(&sync.timeoutLock);


	if ((errnum=pthread_create(&tid,NULL,_func,&sync))!=0){
		exit_on_error(errnum,"failed to create thread");
	}

	struct timespec abstime;
	clock_gettime(CLOCK_MONOTONIC,&abstime);


	abstime.tv_sec += msec/1000;
	abstime.tv_nsec += msec%1000*1000000;
	abstime.tv_sec += abstime.tv_nsec/1000000000;
	abstime.tv_nsec = abstime.tv_nsec%1000000000;

	pthread_mutex_unlock(&sync.triggerLock);

	errnum=pthread_cond_timedwait(&sync.timeout,&sync.timeoutLock,&abstime);
	int status=0;
	if (errnum==0){
	}else if (errnum==ETIMEDOUT){
		pthread_kill(tid,SIGINT);
	}else {
		exit_on_error(errnum,"fatal error");
	}
	pthread_join(tid,(void**)&status);
	return errnum;
}

int Boss::timedcease(int msec)
{
	if(msec<0){
		return cease();
	}else if(msec==0){
		return trycease();
	}else{
		return _timedcease(this,msec);
	}
}
/*----------------------class Employee---------------*/

Employee::Employee()
{
}
Employee::~Employee(){
}
pthread_t* Employee::getID()
{
	return &this->_tid;
}

void Employee::undertake(Boss* boss)
{
	int errnum=0;
	
	if ((errnum=pthread_create(&this->_tid,NULL,Employee::work,boss))!=0){
		exit_on_error(errnum,"failed to create thread!");
	}
}

void* Employee::work(void*arg)
{
	Boss *boss=(Boss*)arg;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	while(true){
		Assignment *as=boss->pickAssignment();
		as->exec();
		delete as;
		pthread_testcancel();
	}
}
	
