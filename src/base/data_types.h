#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include <glog/logging.h>

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

template <typename To, typename From>
inline To implicit_cast(const From& f) {
  return f;
}

namespace detail {
template <bool>
class StaticAssert {
};
}

// like assert.
#define COMPLILE_ASSERT(expr, msg) \
  typedef detail<bool(expr)> msg[bool(expr) ? 1 : -1]

#endif
