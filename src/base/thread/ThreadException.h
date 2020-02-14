#ifndef N_BASE_COMMON_THREAD_EXCEPTION_H
#define N_BASE_COMMON_THREAD_EXCEPTION_H

#include "base/common/Exception.h"
#include "base/time/Time.h"

namespace neptune {
namespace base {

class ThreadSyscallException : public SyscallException
{

 public:
  ThreadSyscallException(const char*, int, int);
  virtual std::string name() const;
  virtual Exception* clone() const;
  virtual void _throw() const;

 private:
  static const char* _name;

};

class ThreadLockedException : public Exception
{

 public:
  ThreadLockedException(const char*, int);
  virtual std::string name() const;
  virtual Exception* clone() const;
  virtual void _throw() const;

 private:
  static const char* _name;
};

class ThreadStartedException : public Exception
{
 public:
  ThreadStartedException(const char*, int);
  virtual std::string name() const;
  virtual Exception* clone() const;
  virtual void _throw() const;

 private:
  static const char* _name;
};

class ThreadNotStartedException : public Exception
{
 public:
  ThreadNotStartedException(const char*, int);
  virtual std::string name() const;
  virtual Exception* clone() const;
  virtual void _throw() const;

 private:
  static const char* _name;
};

class BadThreadControlException : public Exception
{
 public:
  BadThreadControlException(const char*, int);
  virtual std::string name() const;
  virtual Exception* clone() const;
  virtual void _throw() const;

 private:
  static const char* _name;
};

class InvalidTimeoutException : public Exception
{
 public:
  InvalidTimeoutException(const char*, int, const Time&);
  virtual std::string name() const;
  virtual void print(std::ostream&) const;
  virtual Exception* clone() const;
  virtual void _throw() const;

 private:
  Time _timeout;
  static const char* _name;
};

class ThreadCreateException: public Exception
{
 public:
  ThreadCreateException( const char* , int );
  virtual std::string name() const;
  virtual void print(std::ostream&) const;
  virtual Exception* clone() const;
  virtual void _throw() const;
  
 private: 
  static const char* _name;
};

} //namespace base
} //namespace neptune

#endif //N_BASE_COMMON_THREAD_EXCEPTION_H

