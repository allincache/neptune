#ifndef N_DFS_UTIL_BASE_MAIN_H
#define N_DFS_UTIL_BASE_MAIN_H

#include <string>
#include "base/concurrent/Mutex.h"
#include "base/common/Monitor.h"

namespace neptune {
namespace dfs {

class BaseMain
{
 
 public:
  BaseMain();
  virtual ~BaseMain();

  int main(int argc, char* argv[]);

  static BaseMain* instance();    

  int handle_interrupt(int sig);

  /** get work directory*/
  const char* get_work_dir() const;

  /** get log file level*/
  const char* get_log_file_level() const;

  /** get log file path*/
  const char* get_log_path() const;

  /** get log file size*/
  int64_t get_log_file_size() const;

  /** get log file count*/
  int32_t get_log_file_count() const;

  void stop();

  virtual int run(int argc , char* argv[]) = 0;

  /** application parse args*/
  virtual int parse_common_line_args(int argc, char* argv[], std::string& errmsg);

  /** get log file path*/
  virtual const char* get_log_file_path(){return NULL;}

  /** get pid file path*/
  virtual const char* get_pid_file_path(){return NULL;}

  virtual bool destroy() = 0;

  virtual void help();

  virtual void version();

  bool stop_;

  std::string config_file_;
  std::string log_file_path_;
  std::string pid_file_path_;

 private:
  int shutdown();
  int wait_for_shutdown();
  int start(int argc , char* argv[], const bool deamon);

  int initialize_work_dir(const char* app_name);
  int initialize_log_file(const char* app_name);
  int initialize_pid_file(const char* app_name);

 private:
  Monitor<Mutex> monitor_;
  static BaseMain* instance_;
};

} //namespace dfs
} //namespace neptune

#endif
