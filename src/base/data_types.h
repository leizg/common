#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include <glog/logging.h>
#include <google/gflags.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

template<typename To, typename From>
inline To implicit_cast(const From& f) {
  return f;
}

namespace detail {
template<bool>
class StaticAssert {
};
}

// like assert.
#define COMPLILE_ASSERT(expr, msg) \
  typedef detail::StaticAssert<bool(expr)> msg[bool(expr) ? 1 : -1]

#define kMicrosecsPerSecond (1000ULL * 1000)
#define kNanosecsPerMicroSecond (1000ULL)
#define kNanosecsPerSecond (kMicrosecsPerSecond * 1000)

inline timespec timespecFromMicrosecs(uint64 micro_secs) {
  timespec ts = { 0 };
  ts.tv_sec = micro_secs / kMicrosecsPerSecond;
  ts.tv_nsec = micro_secs % kNanosecsPerMicroSecond;
  return ts;
}

inline uint64 microsecsFromTimespec(const timespec ts) {
  uint64 micro_secs = ts.tv_sec * kMicrosecsPerSecond;
  micro_secs += ts.tv_nsec / kNanosecsPerMicroSecond;
  return micro_secs;
}

