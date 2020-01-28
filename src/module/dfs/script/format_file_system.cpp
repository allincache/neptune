#include <stdio.h>
#include "dfs/util/parameter.h"
#include "dfs/dataserver/blockfile_manager.h"
#include "dfs/dataserver/version.h"

using namespace neptune::dfs;
using namespace neptune::dfs::dataserver;
using namespace std;

int main(int argc, char* argv[])
{
  char* conf_file = NULL;
  int help_info = 0;
  int i;
  std::string server_index;
  bool speedup = false;

  while ((i = getopt(argc, argv, "f:i:vhn")) != EOF)
  {
    switch (i)
    {
    case 'f':
      conf_file = optarg;
      break;
    case 'i':
      server_index = optarg;
      break;
    case 'n':
      speedup = true;
      break;
    case 'v':
      fprintf(stderr, "create file system tool, version: %s\n", Version::get_build_description());
      return 0;
    case 'h':
    default:
      help_info = 1;
      break;
    }
  }

  if (conf_file == NULL || server_index.size() == 0 || help_info)
  {
    fprintf(stderr, "\nUsage: %s -f conf_file -i index -h -v\n", argv[0]);
    fprintf(stderr, "  -f configure file\n");
    fprintf(stderr, "  -i server_index  dataserver index number\n");
    fprintf(stderr, "  -n speedup ds start up\n");
    fprintf(stderr, "  -v show version info\n");
    fprintf(stderr, "  -h help info\n");
    fprintf(stderr, "\n");
    return -1;
  }

  int ret = 0;
  NEP_CONFIG.load(conf_file);
  if ((ret = SYSPARAM_FILESYSPARAM.initialize(server_index)) != SUCCESS)
  {
    cerr << "SysParam::load filesystemparam failed:" << conf_file << endl;
    return ret;
  }

  cout << "mount name: " << SYSPARAM_FILESYSPARAM.mount_name_ << " max mount size: "
      << SYSPARAM_FILESYSPARAM.max_mount_size_ << " base fs type: "
      << SYSPARAM_FILESYSPARAM.base_fs_type_ << " superblock reserve offset: "
      << SYSPARAM_FILESYSPARAM.super_block_reserve_offset_ << " main block size: "
      << SYSPARAM_FILESYSPARAM.main_block_size_ << " extend block size: "
      << SYSPARAM_FILESYSPARAM.extend_block_size_ << " block ratio: "
      << SYSPARAM_FILESYSPARAM.block_type_ratio_ << " file system version: "
      << SYSPARAM_FILESYSPARAM.file_system_version_ << " avg inner file size: "
      << SYSPARAM_FILESYSPARAM.avg_segment_size_ << " hash slot ratio: "
      << SYSPARAM_FILESYSPARAM.hash_slot_ratio_ << endl;

  ret = BlockFileManager::get_instance()->format_block_file_system(SYSPARAM_FILESYSPARAM, speedup);
  if (ret)
  {
    fprintf(stderr, "create file system fail. ret: %d\n", ret);
    return ret;
  }
  return 0;
}
