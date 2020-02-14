#ifndef N_DFS_MESSAGE_METANAMESERVERCLIENTMESSAGE_H
#define N_DFS_MESSAGE_METANAMESERVERCLIENTMESSAGE_H

#include "dfs/util/base_packet.h"
#include "dfs/util/meta_server_define.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class BaseMetaParameter: public BasePacket
{
 public:
  BaseMetaParameter();
  virtual ~BaseMetaParameter();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_app_id(const int64_t app_id)
  {
    app_id_ = app_id;
  }
  inline int64_t get_app_id() const
  {
    return app_id_;
  }

  inline void set_user_id(const int64_t user_id)
  {
    user_id_ = user_id;
  }
  inline int64_t get_user_id() const
  {
    return user_id_;
  }

  inline void set_file_path(const char* file_path)
  {
    file_path_ = file_path;
  }
  inline const char* const get_file_path() const
  {
    return file_path_.c_str();
  }
  inline int64_t get_version(void) const { return version_;}
  inline void set_version(const int64_t version) { version_ = version;}
 protected:
  int64_t app_id_;
  int64_t user_id_;
  int64_t version_;
  std::string file_path_;
};

class FilepathActionMessage: public BaseMetaParameter
{
 public:
  FilepathActionMessage();
  virtual ~FilepathActionMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;


  inline void set_new_file_path(const char* new_file_path)
  {
    new_file_path_ = std::string(new_file_path);
  }
  inline const char* get_new_file_path() const
  {
    return new_file_path_.c_str();
  }

  inline void set_action(const MetaActionOp action)
  {
    action_ = action;
  }
  inline MetaActionOp get_action() const
  {
    return action_;
  }

 protected:
  std::string new_file_path_;
  MetaActionOp action_;
};

class WriteFilepathMessage: public BaseMetaParameter
{
 public:
  WriteFilepathMessage();
  virtual ~WriteFilepathMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  inline FragInfo& get_frag_info()
  {
    return frag_info_;
  }
  inline void set_frag_info(const FragInfo& frag_info)
  {
    frag_info_ = frag_info;
  }

 protected:
  FragInfo frag_info_;
};

class ReadFilepathMessage: public BaseMetaParameter
{
 public:
  ReadFilepathMessage();
  virtual ~ReadFilepathMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  inline void set_offset(const int64_t offset)
  {
    offset_ = offset;
  }
  inline int64_t get_offset() const
  {
    return offset_;
  }

  inline void set_size(const int64_t size)
  {
    size_ = size;
  }
  inline int64_t get_size() const
  {
    return size_;
  }

 protected:
  int64_t offset_;
  int64_t size_;
};

class RespReadFilepathMessage: public BasePacket
{
 public:
  RespReadFilepathMessage();
  virtual ~RespReadFilepathMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  inline FragInfo& get_frag_info()
  {
    return frag_info_;
  }
  inline void set_frag_info(const FragInfo& frag_info)
  {
    frag_info_ = frag_info;
  }

  inline void set_still_have(const bool still_have)
  {
    still_have_ = still_have;
  }
  inline bool get_still_have() const
  {
    return still_have_;
  }

 protected:
  FragInfo frag_info_;
  bool still_have_;
};

class LsFilepathMessage: public BaseMetaParameter
{
 public:
  LsFilepathMessage();
  virtual ~LsFilepathMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

    inline void set_file_type(int32_t file_type)
    {
      file_type_ = file_type;
    }
    inline int32_t get_file_type()
    {
      return file_type_;
    }

    inline void set_pid(int64_t pid)
    {
      pid_ = pid;
    }

    inline int64_t get_pid() const
    {
      return pid_;
    }

 private:
  int64_t pid_;
  int32_t file_type_;
};

class RespLsFilepathMessage: public BaseMetaParameter
{
 public:
  RespLsFilepathMessage();
  virtual ~RespLsFilepathMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  inline void set_still_have(bool still_have)
  {
    still_have_ = still_have;
  }

  inline bool get_still_have() const
  {
    return still_have_;
  }

  inline void set_meta_infos(std::vector<MetaInfo>& meta_infos)
  {
    meta_infos_ = meta_infos;
  }

  inline std::vector<MetaInfo>& get_meta_infos()
  {
    return meta_infos_;
  }
  inline int64_t get_version(void) const { return version_;}
  inline void set_version(const int64_t version) { version_ = version;}

 private:
  bool still_have_;
  std::vector<MetaInfo> meta_infos_;
  int64_t version_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_METANAMESERVERCLIENTMESSAGE_H
