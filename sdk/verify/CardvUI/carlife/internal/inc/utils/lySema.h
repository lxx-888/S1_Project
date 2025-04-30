#ifndef __LY_SEMA_H__
#define __LY_SEMA_H__


#include <semaphore.h>


class CLySema
{
public:
	CLySema() {
		sem_init(&sem_, 0, 0);		
	}

	~CLySema() {
		sem_destroy(&sem_);
	}

	void up() {
		sem_post(&sem_);
	}

	void down() {
		sem_wait(&sem_);
	}

private:
	sem_t sem_;
};


#endif
