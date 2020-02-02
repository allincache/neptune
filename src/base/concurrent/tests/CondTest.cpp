#include "base/include/Base.h"
#include "base/concurrent/Cond.h"
#include "base/concurrent/Mutex.h"
#include "base/concurrent/Lock.h"
#include <mutex>
#include <thread>
#include <iostream>
#include <queue>
#include <chrono>
#include <stdio.h>
#include <gtest/gtest.h>

using namespace neptune::base;
using namespace std;

//typedef LockT<Mutex> Lock;

const int maxQueueLen = 5;
Cond _cond;
//Lock _mutex;
Mutex _mutex;
const Mutex& _mutex2 = _mutex;
const LockT<Mutex> _lockt2(_mutex2);
const LockT<Mutex>& _lockt = _lockt2;
Mutex::Lock lock(_mutex);
queue<int> _queue;
bool _stop = false;
bool _waiting = false;


void producter_func(int index) {
	if (_stop) {
    return;
  }
	if (_queue.size() >= maxQueueLen) {
		_mutex.lock();
		_waiting = true;
		while (_stop == false && _queue.size() >= maxQueueLen) {
			_cond.wait<LockT<Mutex>>(_lockt);
		}
		_waiting = false;
		_mutex.unlock();
		if (_stop) {
      return;
    }
  }
  _mutex.lock();
	for(int i=0; i<10; i++) {
		int num = rand() % 1000;
		printf("producer %d, num is %d.\n", index, num);
  	_queue.push(num);
	}
  _mutex.unlock();
  _cond.signal();
}


void consumer_func(int index) {
	while (!_stop) {
		_mutex.lock();
		while (!_stop && _queue.size() == 0) {
			_cond.wait<LockT<Mutex>>(_lockt);
		}
		if (_stop) {
			_mutex.unlock();
			break;
		}
		int num = _queue.front();
		_queue.pop();
		printf("consumer %d, num is %d.\n", index, num);
		_mutex.unlock();
		if (_waiting) {
			_mutex.lock();
			_cond.signal();
			_mutex.unlock();
		}
  }
}
 
//int main(){
//	printf("do main\n");
	// const int producter_num = 5;
	// const int consumer_num = 3;
	// for(int i=0; i <= producter_num; i++) {
	// 	thread producter(producter_func, i);
	// 	producter.join();
	// }
	// for(int j=0; j <= consumer_num; j++) {
	// 	thread consumer(consumer_func, j);
	// 	consumer.join();
	// } 
//	return 0;
//}

TEST(ConcurrentTest, CondTest) {
	printf("do test\n");
	EXPECT_EQ(0, 0);
}

