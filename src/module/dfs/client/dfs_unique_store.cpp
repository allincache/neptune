#include "base/common/Memory.h"
#include "base/common/ErrorMsg.h"
#include "base/fs/FsName.h"
#include "dfs_client_impl.h"
#include "dfs_unique_store.h"

namespace neptune {
namespace dfs {

DfsUniqueStore::DfsUniqueStore() : unique_handler_(NULL)
{
}

DfsUniqueStore::~DfsUniqueStore()
{
  gDelete(unique_handler_);
}

int DfsUniqueStore::initialize(const char* master_addr, const char* slave_addr,
                                const char* group_name, const int32_t area, const char* ns_addr)
{
  int ret = SUCCESS;
  LOG(DEBUG, "init unique handler,  master addr: %s, slave addr: %s, group name: %s, area: %d, ns addr: %s",
            master_addr, slave_addr, group_name, area, ns_addr);
  if (NULL == ns_addr)
  {
    LOG(ERROR, "null ns address");
  }
  else
  {
    ns_addr_ = ns_addr;
    // reuse
    gDelete(unique_handler_);
    unique_handler_ = new TairUniqueHandler();

    if ((ret = static_cast<TairUniqueHandler*>(unique_handler_)
          ->initialize(master_addr, slave_addr, group_name, area)) != SUCCESS)
    {
      LOG(ERROR, "init tair unique handler fail. master addr: %s, slave addr: %s, group name: %s, area: %d",
                master_addr, slave_addr, group_name, area);
    }
  }
  return ret;
}

int64_t DfsUniqueStore::save(const char* buf, const int64_t count,
                              const char* dfs_name, const char* suffix,
                              char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  int ret = ERROR;

  if (NULL == buf || count <= 0)
  {
    LOG(ERROR, "invalie buffer and count. buffer: %p, count: %"PRI64_PREFIX"d", buf, count);
  }
  else if (check_init())
  {
    UniqueKey unique_key;
    UniqueValue unique_value;

    unique_key.data_ = buf;
    unique_key.data_len_ = count;

    UniqueAction action = check_unique(unique_key, unique_value, dfs_name, suffix);
    LOG(DEBUG, "dfs unique store, action: %d", action);

    ret = process(action, unique_key, unique_value, dfs_name, suffix, ret_dfs_name, ret_dfs_name_len);
  }

  return ret != SUCCESS ? INVALID_FILE_SIZE : count;
}

int64_t DfsUniqueStore::save(const char* local_file,
                              const char* dfs_name, const char* suffix,
                              char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  int ret = ERROR;
  int64_t count = INVALID_FILE_SIZE;

  if (check_init())
  {
    char* buf = NULL;
    if ((ret = read_local_file(local_file, buf, count)) != SUCCESS)
    {
      LOG(ERROR, "read local file data fail. ret: %d", ret);
    }
    else
    {
      count = save(buf, count, dfs_name, suffix, ret_dfs_name, ret_dfs_name_len);
    }

    gDelete(buf);
  }

  return count;
}

int32_t DfsUniqueStore::unlink(const char* dfs_name, const char* suffix, int64_t& file_size, const int32_t count)
{
  int32_t ref_count = INVALID_REFERENCE_COUNT;

  if (count <= 0)
  {
    LOG(ERROR, "invalid unlink count: %d", count);
  }
  else if (check_init())
  {
    char* buf = NULL;
    int64_t buf_len = 0;
    int ret = DfsClientImpl::Instance()->fetch_file_ex(buf, buf_len, dfs_name, suffix, ns_addr_.c_str());

    if (ret != SUCCESS)
    {
      LOG(ERROR, "read dfs file data fail. filename: %s, suffix: %s, ret: %d",
                dfs_name, suffix, ret);
    }
    else
    {
      UniqueKey unique_key;
      UniqueValue unique_value;

      unique_key.set_data(buf, buf_len);

      if ((ret = unique_handler_->query(unique_key, unique_value)) != SUCCESS)
      {
        LOG(ERROR, "query unique meta info fail. filename: %s, suffix: %s, ret: %d",
                  dfs_name, suffix, ret);
      }
      else
      {
        // ignore name match, if unlink_count > ref_count, just unlink
        if (unique_value.ref_count_ <= count) // unlink dfs file
        {
          LOG(DEBUG, "refcnt less than unlink count. %d <= %d",unique_value.ref_count_, count);

          // CAUTION: comment this unlink for histroy problem, avoid unlinking file on line,
          // uncomment this unlink for normal use
          //ret = DfsClientImpl::Instance()->unlink(file_size, dfs_name, suffix, ns_addr_.c_str());
          ret = SUCCESS;

          if (ret != SUCCESS)
          {
            LOG(ERROR, "unlink dfs file fail. filename: %s, suffix: %s, ret: %d",
                      dfs_name, suffix, ret);
          }
          else
          {
            ref_count = 0;  // unlink success
          }
        }

        if (SUCCESS == ret)
        {
          // check uniquestore filename and dfs filename.
          // if not match, not modify unique store meta info and file.
          // ... CONSISTENCY ...
          if (!(check_dfsname_match(unique_value.file_name_, dfs_name, suffix)))
          {
            LOG(WARN, "unlink filename mismatch unique store filename: %s%s <> %s",
                      dfs_name, NULL == suffix ? "" : suffix, unique_value.file_name_);
          }
          else                // unlink success and name match, then decrease
          {
            LOG(DEBUG, "unique refcount: %d, decrease count: %d", unique_value.ref_count_, count);
            int32_t bak_ref_count = ref_count;
            // if count >= unqiue_value.ref_count_, will delete this key
            if ((ref_count = unique_handler_->decrease(unique_key, unique_value, count)) < 0)
            {
              // if dfs file is already unlinked(ref_count = 0), ignore error.
              if (0 == bak_ref_count) // not unlink file, must fail
              {
                ref_count = 0;
              }
              LOG(ERROR, "decrease count fail. count: %d", count);
            }
            else
            {
              file_size = buf_len; // decrease success, set file size
            }
          }
        }
      }

      gDelete(buf);
    }
  }
  return ref_count;
}

bool DfsUniqueStore::check_init()
{
  bool ret = true;
  if (NULL == unique_handler_)
  {
    LOG(ERROR, "unique handler not init");
    ret = false;
  }
  return ret;
}

bool DfsUniqueStore::check_name_match(const char* orig_dfs_name, const char* dfs_name, const char* suffix)
{
  bool ret = false;

  if (NULL == dfs_name || '\0' == dfs_name[0]) // no dfs name
  {
    // check suffix
    ret = check_suffix_match(orig_dfs_name, suffix);
  }
  else
  {
    // has dfs name, must explicitly check
    ret = check_dfsname_match(orig_dfs_name, dfs_name, suffix);
  }
  return ret;
}

bool DfsUniqueStore::check_suffix_match(const char* orig_dfs_name, const char* suffix)
{
  bool ret = true;
  if (suffix != NULL && suffix[0] != '\0')
  {
    // historically, only check whether tailing STANDARD_SUFFIX_LEN chars in suffix match.
    // just like this:
    //
    // int32_t orig_name_len = strlen(orig_dfs_name);
    // int32_t suffix_len = strlen(suffix);
    // const char* orig_suffix = orig_dfs_name + FILE_NAME_LEN;

    // if (strlen(orig_dfs_name + FILE_NAME_LEN) > STANDARD_SUFFIX_LEN)
    // {
    //   orig_suffix = orig_dfs_name + (orig_name_len - STANDARD_SUFFIX_LEN);
    
    // if (suffix_len > STANDARD_SUFFIX_LEN)
    // {
    //   suffix += suffix_len - STANDARD_SUFFIX_LEN;
    
    // ret = strncmp(orig_suffix, suffix, STANDARD_SUFFIX_LEN) == 0 ? true : false;
    //
    // for clean meanning, change it.
    // unqiue store suffix, so just string compare.
    // not strncmp.
    ret = strcmp(orig_dfs_name + FILE_NAME_LEN, suffix) == 0 ? true : false;
  }
  return ret;
}

bool DfsUniqueStore::check_dfsname_match(const char* orig_dfs_name, const char* dfs_name, const char* suffix)
{
  bool ret = false;
  FSName orig_name(orig_dfs_name);
  FSName cur_name(dfs_name, suffix);
  if (orig_name.get_block_id() == cur_name.get_block_id() &&
      orig_name.get_file_id() == cur_name.get_file_id())
  {
    ret = true;
  }

  return ret;
}

UniqueAction DfsUniqueStore::check_unique(UniqueKey& unique_key, UniqueValue& unique_value,
                                          const char* dfs_name, const char* suffix)
{
  int ret = ERROR;
  UniqueAction action = UNIQUE_ACTION_NONE;

  if ((ret = unique_handler_->query(unique_key, unique_value)) != SUCCESS)
  {
    if (ret == EXIT_UNIQUE_META_NOT_EXIST) // not exist, save data and meta
    {
      action = UNIQUE_ACTION_SAVE_DATA_SAVE_META;
      LOG(INFO, "unique info not exist, ret: %d", ret);
    }
    else                    // query fail, just save save data
    {
      action = UNIQUE_ACTION_SAVE_DATA;
      LOG(INFO, "query unique meta info fail, ret: %d", ret);
    }
  }
  else if (!check_name_match(unique_value.file_name_, dfs_name, suffix))
  {
    action = UNIQUE_ACTION_SAVE_DATA; // suffix or file name not match, save data
    LOG(WARN, "filename not match. unique filename: %s, dfsname: %s, suffix: %s",
              unique_value.file_name_, dfs_name, suffix);
  }
  else                      // unique info exist and suffix match
  {
    LOG(DEBUG, "unique meta found and name match: filename: %s, refcnt: %d, version: %d", unique_value.file_name_, unique_value.ref_count_, unique_value.version_);
    DfsFileStat file_stat;
    ret = DfsClientImpl::Instance()->stat_file(&file_stat, unique_value.file_name_, NULL,
                                            NORMAL_STAT, ns_addr_.c_str());

    if (ret != SUCCESS)
    {
      LOG(WARN, "dfs file is not normal status, resave. ret: %d, filename: %s",
                ret, unique_value.file_name_);
      action = UNIQUE_ACTION_SAVE_DATA_UPDATE_META;
    }
    else if (file_stat.size_ != unique_key.data_len_)
    {
      LOG(WARN, "dfs file size conflict: %"PRI64_PREFIX"d <> %d", file_stat.size_, unique_key.data_len_);
      action = UNIQUE_ACTION_SAVE_DATA_UPDATE_META;
      // unlink this dirty file?
    } // else if check crc?
    else
    {
      action = UNIQUE_ACTION_UPDATE_META; // just update unique meta
    }
  }

  return action;
}

int DfsUniqueStore::process(UniqueAction action, UniqueKey& unique_key, UniqueValue& unique_value,
                            const char* dfs_name, const char* suffix,
                            char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  int ret = SUCCESS;

  // first check if write data
  switch (action)
  {
  case UNIQUE_ACTION_SAVE_DATA:
    ret = save_data(unique_key, dfs_name, suffix, ret_dfs_name, ret_dfs_name_len);
    break;
  case UNIQUE_ACTION_SAVE_DATA_SAVE_META:
    ret = save_data_save_meta(unique_key, unique_value, dfs_name, suffix, ret_dfs_name, ret_dfs_name_len);
    break;
  case UNIQUE_ACTION_SAVE_DATA_UPDATE_META:
    ret = save_data_update_meta(unique_key, unique_value, dfs_name, suffix, ret_dfs_name, ret_dfs_name_len);
    break;
  case UNIQUE_ACTION_UPDATE_META:
    ret = update_meta(unique_key, unique_value, ret_dfs_name, ret_dfs_name_len);
    break;
  default:
    LOG(ERROR, "unkown action: %d", action);
    break;
  }
  return ret;
}

int DfsUniqueStore::save_data(UniqueKey& unique_key,
                              const char* dfs_name, const char* suffix,
                              char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  int ret = DfsClientImpl::Instance()->
    save_buf_ex(ret_dfs_name, ret_dfs_name_len, unique_key.data_, unique_key.data_len_, T_DEFAULT,
                  dfs_name, suffix, ns_addr_.c_str()) < 0 ? ERROR : SUCCESS;

  LOG(DEBUG, "write dfs data ret: %d, name: %s", ret, ret != SUCCESS ? "NULL" : ret_dfs_name);
  if (ret != SUCCESS)
  {
    LOG(ERROR, "save data to dfs fail. dfsname: %s, suffix: %s, ret: %d",
              dfs_name, suffix, ret);
  }
  return ret;
}

int DfsUniqueStore::save_data_save_meta(UniqueKey& unique_key, UniqueValue& unique_value,
                                        const char* dfs_name, const char* suffix,
                                        char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  int ret = save_data(unique_key, dfs_name, suffix, ret_dfs_name, ret_dfs_name_len);

  if (SUCCESS == ret)
  {
    unique_value.set_file_name(ret_dfs_name, suffix);
    // first insert, set magic version to avoid dirty insert when concurrency occure
    unique_value.version_ = UNIQUE_FIRST_INSERT_VERSION;
    unique_value.ref_count_ = 1;

    ret = unique_handler_->insert(unique_key, unique_value);
    // save unique meta info fail. just ignore
    if (ret != SUCCESS)
    {
      LOG(WARN, "save unique meta info fail, ignore. filename: %s, ret: %d", unique_value.file_name_, ret);
      ret = SUCCESS;
    }
  }

  return ret;
}

int DfsUniqueStore::save_data_update_meta(UniqueKey& unique_key, UniqueValue& unique_value,
                                          const char* dfs_name, const char* suffix,
                                          char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  int ret = save_data(unique_key, dfs_name, suffix, ret_dfs_name, ret_dfs_name_len);

  if (SUCCESS == ret)
  {
    LOG(DEBUG, "update meta info, filename: %s, refcnt: %d, version: %d", ret_dfs_name, unique_value.ref_count_, unique_value.version_);
    unique_value.set_file_name(ret_dfs_name, suffix);

    int32_t ref_count = unique_handler_->increase(unique_key, unique_value);
    if (ref_count < 0)
    {
      LOG(WARN, "update unique meta info fail, ignore. filename: %s, refcnt: %d, version: %d, ret: %d",
                unique_value.file_name_, unique_value.ref_count_, unique_value.version_, ret);
      // just ignore
    }
  }

  return ret;
}

int DfsUniqueStore::update_meta(UniqueKey& unique_key, UniqueValue& unique_value,
                                char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  int ret = wrap_file_name(unique_value.file_name_, ret_dfs_name, ret_dfs_name_len);

  if (ret != SUCCESS)
  {
    LOG(ERROR, "return name fail. ret: %d", ret);
  }
  else
  {
    LOG(DEBUG, "update meta info, refcnt: %d, version: %d", unique_value.ref_count_, unique_value.version_);
    int32_t ref_count = unique_handler_->increase(unique_key, unique_value);
    if (ref_count < 0)
    {
      LOG(WARN, "update unique meta info fail, ignore. filename: %s, refcnt: %d, version: %d, ret: %d",
                unique_value.file_name_, unique_value.ref_count_, unique_value.version_, ret);
      // just ignore
    }
  }

  return ret;
}

int DfsUniqueStore::wrap_file_name(const char* dfs_name, char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  int ret = ERROR;

  if (ret_dfs_name != NULL)
  {
    if (ret_dfs_name_len < DFS_FILE_LEN)
    {
      LOG(ERROR, "name buffer length less: %d < %d", ret_dfs_name_len, DFS_FILE_LEN);
    }
    else
    {
      // history reason. unique store file name has suffix,
      // and maybe heading FILE_NAME_LEN chars have no suffix encoded,
      // so must reencode to get name
      FSName fsname(dfs_name);
      if (fsname.is_valid())
      {
        memcpy(ret_dfs_name, fsname.get_name(), DFS_FILE_LEN);
        ret = SUCCESS;
      }
    }
  }
  return ret;
}

int64_t DfsUniqueStore::get_local_file_size(const char* local_file)
{
  int64_t file_size = 0;
  if (NULL == local_file)
  {
    LOG(ERROR, "null local file");
  }
  else
  {
    struct stat file_stat;
    if (stat(local_file, &file_stat) < 0)
    {
      LOG(ERROR, "stat local file %s fail, error: %s", local_file, strerror(errno));
    }
    else
    {
      file_size = file_stat.st_size;
    }
  }
  return file_size;
}

int DfsUniqueStore::read_local_file(const char* local_file, char*& buf, int64_t& count)
{
  int ret = ERROR;
  int fd = -1;
  int64_t file_length = get_local_file_size(local_file);

  if (file_length <= 0)
  {
    LOG(ERROR, "get local file %s size fail.", local_file);
  }
  else if (file_length > DFS_MALLOC_MAX_SIZE)
  {
    LOG(ERROR, "file length larger than max malloc size. %"PRI64_PREFIX"d > %"PRI64_PREFIX"d",
              file_length, DFS_MALLOC_MAX_SIZE);
  }
  else if ((fd = ::open(local_file, O_RDONLY)) < 0)
  {
    LOG(ERROR, "open local file %s fail, error: %s", local_file, strerror(errno));
  }
  else
  {
    // use MUST free
    buf = new char[file_length];
    int32_t read_len = 0, alread_read_len = 0;

    while (1)
    {
      read_len = ::read(fd, buf + alread_read_len, MAX_READ_SIZE);
      if (read_len < 0)
      {
        LOG(ERROR, "read file %s data fail. error: %s", local_file, strerror(errno));
        break;
      }

      alread_read_len += read_len;
      if (alread_read_len >= file_length)
      {
        ret = SUCCESS;
        count = alread_read_len;
        break;
      }
    }

    if (ret != SUCCESS)
    {
      gDeleteA(buf);
    }

    ::close(fd);
  }

  return ret;
}

}
}
