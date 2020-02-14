#include "net.h"

namespace neptune {
namespace base {

StatCounter StatCounter::_gStatCounter;

StatCounter::StatCounter() {
  clear();
}

StatCounter::~StatCounter() {
}

void StatCounter::log() {
  //LOG(INFO, "_packetReadCnt: %u, _packetWriteCnt: %u, _dataReadCnt: %u, _dataWriteCnt: %u",
  //          _packetReadCnt, _packetWriteCnt, _dataReadCnt, _dataWriteCnt);
}

void StatCounter::clear() {
  _packetReadCnt = 0;
  _packetWriteCnt = 0;
  _dataReadCnt = 0;
  _dataWriteCnt = 0;
}

}//namespace base
}//namespace neptune
