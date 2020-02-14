#ifndef PROFILER_H
#define PROFILER_H

#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <stdint.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <errno.h>
#include "base/include/Base.h"

#define PROFILER_START(s) do { if(util::Profiler::m_profiler.status == 1) util::Profiler::m_profiler.start((s)); } while(0)
#define PROFILER_STOP() do { if(util::Profiler::m_profiler.status == 1) util::Profiler::m_profiler.reset(); } while (0)
#define PROFILER_BEGIN(s) do { if(util::Profiler::m_profiler.status == 1) util::Profiler::m_profiler.begin((s)); } while(0)
#define PROFILER_END() do { if(util::Profiler::m_profiler.status == 1) util::Profiler::m_profiler.end(); } while(0)
#define PROFILER_DUMP() do { if(util::Profiler::m_profiler.status == 1) util::Profiler::m_profiler.dump(); } while(0)
#define PROFILER_SET_THRESHOLD(sh) util::Profiler::m_profiler.threshold = (sh)
#define PROFILER_SET_STATUS(st) util::Profiler::m_profiler.status = (st)

using namespace neptune::base;

namespace neptune {
namespace dfs {

// template<class T>
// class ThreadLocal {
//  public:
// 	ThreadLocal () {
// 	    pthread_key_create(&key, NULL);
// 	}
// 	virtual ~ThreadLocal () {}
// 	T get() { return (T)pthread_getspecific(key); }
// 	void set(T data) { pthread_setspecific(key, (void *)data); }

//  private:
// 	pthread_key_t key;
// };

class Entry
{
 public:
	Entry(const std::string& message, Entry *parent, Entry *first) {
		this->message = message;
		this->parent = parent;
		this->first = (first == NULL) ? this : first;
		btime = (first == NULL) ? 0 : first->stime;
		stime = Entry::getTime();
		etime = 0;
	}

	~Entry() {
	  if (!subEntries.empty())
		for (size_t i=0; i<subEntries.size(); i++) 
		  delete subEntries[i];
	}

	long getStartTime() {
	  return (btime > 0) ? (stime - btime) : 0;
	}

	long getDuration() {
	  if (etime >= stime)
			return (etime - stime);
	  else
			return -1;
	}

	long getEndTime() {
	  if (etime >= btime)
			return (etime - btime);
	  else
			return -1;
	}

	long getMyDuration() {
	  long td = getDuration();
	  if (td < 0)
			return -1;
	  else if (subEntries.empty())
			return td;
	  else {
			for (size_t i=0; i<subEntries.size(); i++)
		  	td -= subEntries[i]->getDuration();
			return (td < 0) ? -1 : td;
	  }
	}

	double getPercentage() {
	  double pd = 0;
	  double d = getMyDuration();

	  if (!subEntries.empty())
			pd = getDuration();
	  else if (parent && parent->isReleased())
			pd = static_cast<double>(parent->getDuration());
	  if (pd > 0 && d > 0)
			return d / pd;
	  return 0;
	}

	double getPercentageOfTotal() {
	  double fd = 0;
	  double d = getDuration();

	  if (first && first->isReleased())
			fd = static_cast<double>(first->getDuration());
	  if (fd > 0 && d > 0)
			return d / fd;
	  return 0;
	}

	void release() {
	  etime = Entry::getTime();
	}

	bool isReleased() {
	  return etime > 0;
	}

	void doSubEntry(const std::string& message) {
	  Entry *entry = new Entry(message, this, first);
	  subEntries.push_back(entry);
	}

	Entry *getUnreleasedEntry() {
	  if(subEntries.empty()) 
			return NULL;
	  Entry *se = subEntries.back();
	  if(se->isReleased())
			return NULL;
	  else
			return se;
	}

  std::string toString() {
	  return toString("", "");
	}

  std::string toString(const std::string& pre1, const std::string& pre2) {
    std::ostringstream ss;
	  toString(pre1, pre2, ss);
	  return ss.str();
	}

  std::string toString(const std::string& pre1, const std::string& pre2, std::ostringstream &ss) {
	  ss<<pre1;
	  if (isReleased()) {
			char temp[256];
			sprintf(temp, "%lu [%lu(us), %lu(us), %.2f%%, %.2f%%] - %s", getStartTime(), getDuration(), getMyDuration(), getPercentage() * 100, getPercentageOfTotal() * 100, message.c_str());
			ss<<temp;
	  } else {
			ss<<"[UNRELEASED]";
	  }
	  for (size_t i=0; i<subEntries.size(); i++) {
			Entry *ent = subEntries[i];
			ss<<std::endl;

			if (i == 0)
		    ss<<ent->toString(pre2 + "+---", pre2 + "|  ");
			else if (i == (subEntries.size() - 1))
		    ss<<ent->toString(pre2 + "`---", pre2 + "    ");
			else
		    ss<<ent->toString(pre2 + "+---", pre2 + "|   ");
	  }
		return ss.str();
	}

	static uint64_t getTime() {
	  timeval time;
	  gettimeofday(&time, NULL);
	  return time.tv_sec * 1000000 + time.tv_usec;
	}

  std::string message;
 private:
  std::vector<Entry *> subEntries;
	Entry *parent;
	Entry *first;
	uint64_t stime;
	uint64_t btime;
	uint64_t etime;
};

class Profiler
{
 public:
	Profiler();

	void start(const std::string& description);
	void stop();
	void reset();
	void begin(const std::string& description);
	void end();
	long getDuration();
	Entry *getCurrentEntry();
	void dump();
 
 private:
	ThreadLocal<Entry*> entry;
 
 public:
	int threshold;
	int status;
	static Profiler m_profiler;
};

}//namespace base
}//namespace neptune

#endif
