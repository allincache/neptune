#ifndef N_BASE_THREAD_LOCAL_H
#define N_BASE_THREAD_LOCAL_H

namespace neptune {
namespace base {

template<class T>
class ThreadLocal {
 public:
	ThreadLocal () {
	  pthread_key_create(&key, NULL);
	}
	virtual ~ThreadLocal () {}
	T get() { return (T)pthread_getspecific(key); }
	void set(T data) { pthread_setspecific(key, (void *)data); }

 private:
	pthread_key_t key;
};

}
}
#endif