#include <iostream>
#include <sys/time.h>
#include "base/thread/CThread.h"


using namespace neptune::base;

class Test : public Runnable 
{
 public:
  Test() { num = 0; }
  virtual ~Test() { num = 0; }
	void run(CThread *thread, void * arg);
 
 private:
	int num;
};
 
void Test::run(CThread *thread, void* arg)
{
	do {
		++num;
	} while (num < 1000);
	std::cout << num << std::endl;
  return;
}

int main() {
  Test t1;
	CThread *t = new CThread();
	t->start(&t1, NULL);
	std::cout << "OK." << std::endl;
	t->join();
	sleep(1);
	Test t2;
	t->start(&t2, NULL);
	t->join();
	sleep(1);
  return 0;
}