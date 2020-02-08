#include <unistd.h>
#include "base/log/Log.h"

using namespace neptune::base;

int main(int argc, char *argv[])
{
  LOG(INFO, "xxx: %s:%d", "xxxx", 1);
  LOG(ERROR, "xxx: %s:%d", "xxxx", 1);
  LOGGER.setFileName("/tmp/test.txt");
  for(int i=0; i<50; i++) {
      LOG(ERROR, "xxx: %s:%d", "xxxx", i);
      LOG(WARN, "xxx: %s:%d", "xxxx", i);
      LOG(INFO, "xxx: %s:%d", "xxxx", i);
      LOG(DEBUG, "xxx: %s:%d", "xxxx", i);
      //getchar();
  }
  //test rotateLog()
  CLogger logger;
  logger.setFileName("/tmp/test.log", false, true);
  logger.setLogLevel("INFO");
  logger.setMaxFileIndex(100);
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 50; j++)
    {
      logger.logMessage(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__,
        pthread_self(), "test rotateLog(): %d", j);
    }
    logger.rotateLog(NULL);
    sleep(2);
  }

  return 0;
}