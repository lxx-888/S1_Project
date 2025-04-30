#ifndef __LY_SINGLE_TASK_H__
#define __LY_SINGLE_TASK_H__


#include <pthread.h>


class CLySingleTask
{
protected:
	CLySingleTask()
	{
		pthread_mutex_init(&mutex, NULL);
	}
	
	~CLySingleTask()
	{
		pthread_mutex_destroy(&mutex);
	}
protected:	
	inline void lock()
	{
		pthread_mutex_lock(&mutex);
	}

	inline void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}
private:
	pthread_mutex_t mutex;
};


#endif
