#ifndef NEP_COMMON_DEFINE_H_
#define NEP_COMMON_DEFINE_H_

#ifndef UNUSED
#define UNUSED(v) ((void)(v))
#endif

#include <stdint.h>
#include <stdlib.h>

namespace neptune {
namespace base {

// wrap cdefin.h into namespace.
// undef maroc to reserve global cdefine's namespace
#undef NEP_COMMON_CDEFINE_H_
#include "CDefine.h"
#undef NEP_COMMON_CDEFINE_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                  \
  void operator=(const TypeName&)

} //namespace base
} //namespace neptune

#endif //NEP_COMMON_DEFINE_H_
