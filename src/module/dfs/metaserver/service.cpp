#include <exception>
#include "dfs/util/dfs.h"
#include "base/common/Memory.h"
#include "metaserver.h"
#include "block_collect.h"

int main(int argc, char* argv[])
{
  neptune::dfs::metaserver::MetaServer service;
  return service.main(argc, argv);
}

