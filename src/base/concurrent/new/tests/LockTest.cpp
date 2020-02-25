#include <stdlib.h>
#include "base/concurrent/new/Lock.h"
#include "base/thread/new/PosixThread.h"
#include <gtest/gtest.h>

namespace base {


class BasicLockTestThread : public PosixThread::Delegate {
 public:
  explicit BasicLockTestThread(Lock* lock) : lock_(lock), acquired_(0) {}

  void ThreadMain() override {
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      lock_->Release();
    }
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      //PosixThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
      lock_->Release();
    }
    for (int i = 0; i < 10; i++) {
      if (lock_->Try()) {
        acquired_++;
        //PosixThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
        lock_->Release();
      }
    }
  }

  int acquired() const { return acquired_; }

 private:
  Lock* lock_;
  int acquired_;

  //DISALLOW_COPY_AND_ASSIGN(BasicLockTestThread);
};

TEST(LockTest, Basic) {
  Lock lock;
  BasicLockTestThread thread(&lock);
  PosixThreadHandle handle;

  ASSERT_TRUE(PosixThread::Create(0, &thread, &handle));

  int acquired = 0;
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    lock.Release();
  }
  for (int i = 0; i < 10; i++) {
    lock.Acquire();
    acquired++;
    //PosixThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
    lock.Release();
  }
  for (int i = 0; i < 10; i++) {
    if (lock.Try()) {
      acquired++;
      //PosixThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
      lock.Release();
    }
  }
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    //PosixThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
    lock.Release();
  }

  PosixThread::Join(handle);

  EXPECT_GE(acquired, 20);
  EXPECT_GE(thread.acquired(), 20);
}

int main(int argc, char **argv)
{
 testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
}


}  // namespace base
