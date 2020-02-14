#include <exception>
#include "dfs/util/dfs.h"
#include "base/common/Memory.h"
#include "dataservice.h"

int main(int argc, char* argv[])
{
  neptune::dfs::dataserver::DataService service;
  return service.main(argc, argv);
}

