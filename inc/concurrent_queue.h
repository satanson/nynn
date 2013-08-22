#ifndef CONCURRENT_QUEUE_H_BY_SATANSON
#define CONCURRENT_QUEUE_H_BY_SATANSON


#include<public.h>
#include<list>
using namespace std;

template <typename T> class concurrent_queue{
	private:
		list<T> lst;
		pthread_mutex_t lock;
		pthread_cond_t notempty;
	public:
		concurrent_queue(){
			pthread_mutex_init(&this->lock,NULL);
			pthread_cond_init(&this->notempty,NULL);
		}
		~concurrent_queue(){
			pthread_mutex_destroy(&this->lock);
			pthread_cond_destroy(&this->notempty);
		}
		T pop(){
			T item;
			pthread_mutex_lock(&this->lock);
			while(lst.empty()){
				pthread_cond_wait(&this->notempty,&this->lock);
			}
			item=lst.front();
			lst.pop_front();
			pthread_mutex_unlock(&this->lock);
			return item;
		}
		void push(const T& item){
			pthread_mutex_lock(&this->lock);
			lst.push_back(item);
			pthread_mutex_unlock(&this->lock);
			pthread_cond_broadcast(&this->notempty);
		}
		bool empty(){
			bool ret;
			pthread_mutex_lock(&this->lock);
			ret=lst.empty();
			pthread_mutex_unlock(&this->lock);
			return ret;
		}
};
#endif

