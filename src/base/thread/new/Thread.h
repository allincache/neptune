#ifndef BASE_THREADING_THREAD_H_
#define BASE_THREADING_THREAD_H_

#include <stddef.h>
#include <memory>
#include <string>
#include "base/thread/new/PosixThread.h"


namespace base {

//class MessagePump;
//class RunLoop;

// IMPORTANT: Instead of creating a base::Thread, consider using
// base::Create(Sequenced|SingleThread)TaskRunnerWithTraits().
//
// A simple thread abstraction that establishes a MessageLoop on a new thread.
// The consumer uses the MessageLoop of the thread to cause code to execute on
// the thread.  When this object is destroyed the thread is terminated.  All
// pending tasks queued on the thread's message loop will run to completion
// before the thread is terminated.
//
// WARNING! SUBCLASSES MUST CALL Stop() IN THEIR DESTRUCTORS!  See ~Thread().
//
// After the thread is stopped, the destruction sequence is:
//
//  (1) Thread::CleanUp()
//  (2) MessageLoop::~MessageLoop
//  (3.b) MessageLoopCurrent::DestructionObserver::WillDestroyCurrentMessageLoop
//
// This API is not thread-safe: unless indicated otherwise its methods are only
// valid from the owning sequence (which is the one from which Start() is
// invoked -- should it differ from the one on which it was constructed).
//
// Sometimes it's useful to kick things off on the initial sequence (e.g.
// construction, Start(), task_runner()), but to then hand the Thread over to a
// pool of users for the last one of them to destroy it when done. For that use
// case, Thread::DetachFromSequence() allows the owning sequence to give up
// ownership. The caller is then responsible to ensure a happens-after
// relationship between the DetachFromSequence() call and the next use of that
// Thread object (including ~Thread()).
class Thread : PosixThread::Delegate {
 public:
  struct Options {
    //typedef Callback<std::unique_ptr<MessagePump>()> MessagePumpFactory;

    Options();
    //Options(MessageLoop::Type type, size_t size);
    Options(const Options& other);
    ~Options();

    // Specifies the type of message loop that will be allocated on the thread.
    // This is ignored if message_pump_factory.is_null() is false.
    //MessageLoop::Type message_loop_type = MessageLoop::TYPE_DEFAULT;

    // Specifies timer slack for thread message loop.
    //TimerSlack timer_slack = TIMER_SLACK_NONE;

    // Used to create the MessagePump for the MessageLoop. The callback is Run()
    // on the thread. If message_pump_factory.is_null(), then a MessagePump
    // appropriate for |message_loop_type| is created. Setting this forces the
    // MessageLoop::Type to TYPE_CUSTOM.
    //MessagePumpFactory message_pump_factory;

    // Specifies the maximum stack size that the thread is allowed to use.
    // This does not necessarily correspond to the thread's initial stack size.
    // A value of 0 indicates that the default maximum should be used.
    size_t stack_size = 0;

    // Specifies the initial thread priority.
    ThreadPriority priority = ThreadPriority::NORMAL;

    // If false, the thread will not be joined on destruction. This is intended
    // for threads that want TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN
    // semantics. Non-joinable threads can't be joined (must be leaked and
    // can't be destroyed or Stop()'ed).
    // TODO(gab): allow non-joinable instances to be deleted without causing
    // user-after-frees (proposal @ https://crbug.com/629139#c14)
    bool joinable = true;
  };

  explicit Thread(const std::string& name);

  ~Thread() override;

  bool Start();

  bool StartWithOptions(const Options& options);

  bool WaitUntilThreadStarted() const;

  void Stop();

  void StopSoon();

  void DetachFromSequence();

  const std::string& thread_name() const { return name_; }

  PosixThreadId GetThreadId() const;

  bool IsRunning() const;

 protected:
  // Called just prior to starting the message loop
  virtual void Init() {}

  // Called to start the run loop
//  virtual void Run(RunLoop* run_loop);

  // Called just after the message loop ends
  virtual void CleanUp() {}

  static void SetThreadWasQuitProperly(bool flag);
  static bool GetThreadWasQuitProperly();

  // Bind this Thread to an existing MessageLoop instead of starting a new one.
  // TODO(gab): Remove this after ios/ has undergone the same surgery as
  // BrowserThreadImpl (ref.
  // https://chromium-review.googlesource.com/c/chromium/src/+/969104).
  //void SetMessageLoop(MessageLoop* message_loop);

  bool using_external_message_loop() const {
    return using_external_message_loop_;
  }

 private:
  // Friends for message_loop() access:
  //friend class MessageLoopTaskRunnerTest;
  //friend class ScheduleWorkTest;
  //friend class MessageLoopTaskRunnerTest;

  // PosixThread::Delegate methods:
  void ThreadMain() override;

  void ThreadQuitHelper();

  // Mirrors the Options::joinable field used to start this thread. Verified
  // on Stop() -- non-joinable threads can't be joined (must be leaked).
  bool joinable_ = true;

  // If true, we're in the middle of stopping, and shouldn't access
  // |message_loop_|. It may non-nullptr and invalid.
  // Should be written on the thread that created this thread. Also read data
  // could be wrong on other threads.
  bool stopping_ = false;

  // True while inside of Run().
  bool running_ = false;
  //mutable base::Lock running_lock_;  // Protects |running_|.

  // The thread's handle.
  PosixThreadHandle thread_;
  //mutable base::Lock thread_lock_;  // Protects |thread_|.

  // The thread's id once it has started.
  PosixThreadId id_ = kInvalidThreadId;
  // Protects |id_| which must only be read while it's signaled.
  //mutable WaitableEvent id_event_;

  // The thread's MessageLoop and RunLoop. Valid only while the thread is alive.
  // Set by the created thread.
  //MessageLoop* message_loop_ = nullptr;
  //RunLoop* run_loop_ = nullptr;

  // True only if |message_loop_| was externally provided by |SetMessageLoop()|
  // in which case this Thread has no underlying |thread_| and should merely
  // drop |message_loop_| on Stop(). In that event, this remains true after
  // Stop() was invoked so that subclasses can use this state to build their own
  // cleanup logic as required.
  bool using_external_message_loop_ = false;

  // Stores Options::timer_slack_ until the sequence manager has been bound to
  // a thread.
  //TimerSlack timer_slack_ = TIMER_SLACK_NONE;

  // The name of the thread.  Used for debugging purposes.
  const std::string name_;

  // Signaled when the created thread gets ready to use the message loop.
  //mutable WaitableEvent start_event_;

  // This class is not thread-safe, use this to verify access from the owning
  // sequence of the Thread.
  //SequenceChecker owning_sequence_checker_;

  //DISALLOW_COPY_AND_ASSIGN(Thread);
};

}  // namespace base

#endif  // BASE_THREADING_THREAD_H_
