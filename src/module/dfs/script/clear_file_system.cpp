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

  while ((i = getopt(argc, argv, "f:i:vh")) != EOF)
  {
    switch (i)
    {
    case 'f':
      conf_file = optarg;
      break;
    case 'i':
      server_index = optarg;
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

  if ((conf_file == NULL) || (server_index.size() == 0) || help_info)
  {
    fprintf(stderr, "\nUsage: %s -f conf_file -i server_index -h -v\n", argv[0]);
    fprintf(stderr, "  -f configure file\n");
    fprintf(stderr, "  -i server_index  dataserver index number\n");
    fprintf(stderr, "  -v show version info\n");
    fprintf(stderr, "  -h help info\n");
    fprintf(stderr, "\n");
    return -1;
  }

  int ret = 0;
  if (EXIT_SUCCESS != NEP_CONFIG.load(conf_file))
  {
    cerr << "load config error conf_file is " << conf_file;
    return ERROR;
  }
  if ((ret = SYSPARAM_DATASERVER.initialize(conf_file, server_index)) != SUCCESS)
  {
    cerr << "SysParam::load file system param failed:" << conf_file << endl;
    return ret;
  }

  ret = BlockFileManager::get_instance()->clear_block_file_system(SYSPARAM_FILESYSPARAM);
  if (ret)
  {
    fprintf(stderr, "clear file system fail. ret: %d, error: %d, desc: %s\n", ret, errno, strerror(errno));
    return ret;
  }
  return 0;
}
