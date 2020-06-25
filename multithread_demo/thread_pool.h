#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <deque>
#include "locker.h"
#include <iostream>

template <typename T>
class ThreadPool {
public:
	ThreadPool(int thread_number);
	~ThreadPool();
	void add(T*);
private:
	static void* worker(void * arg);
	std::deque<T*> q;
	pthread_t *threads;
	int thread_number;
	void run();
	bool stop;
	sem* queue_sem;
	locker* queue_mutex;
};

template <typename T>
void ThreadPool<T>::add(T* req) {
	queue_mutex->lock();
	q.push_back(req);
	queue_mutex->unlock();
	queue_sem->post();
}

template <typename T>
ThreadPool<T>::ThreadPool(int thread_number) {
	assert(thread_number > 0 && thread_number <= 8);
	this->thread_number = thread_number;
	stop = false;
	queue_sem = new sem();
	queue_mutex = new locker();
	threads = new pthread_t(thread_number);
	for(int i = 0; i < thread_number; i++) {
		pthread_create(threads + i, NULL, worker, this);
		pthread_detach(threads[i]);
	}
}

template <typename T>
void* ThreadPool<T>::worker(void *arg) {
	ThreadPool* pool = (ThreadPool*)arg;
	pool->run();
	return pool;
}

template <typename T>
void ThreadPool<T>::run() {
	while(!stop) {
		queue_sem->wait();
		queue_mutex->lock();
		if (q.empty()) {
			queue_mutex->unlock();
			continue;
		}
		T* front = q.front();
		q.pop_front();
		queue_mutex->unlock();
		if (!front) continue;
		front->handle();
	}
}


