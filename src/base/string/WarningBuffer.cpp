#include "WarningBuffer.h"

namespace neptune {
namespace base {

bool WarningBuffer::is_log_on_ = false;

WarningBuffer *get_tsi_warning_buffer()
{
  static WarningBufferFactory instance;
  return instance.get_buffer();
}

} //namespace base
} //namespace neptune

