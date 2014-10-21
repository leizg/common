#ifndef THREAD_UTIL_H_
#define THREAD_UTIL_H_

#include "macro_def.h"
#include "data_types.h"

#include <pthread.h>

class Mutex {
 public:
  Mutex() {
    int ret = ::pthread_mutex_init(&mutex_, NULL);
    CHECK_EQ(ret, 0)<< "pthread_mutex_init error: " << strerror(ret);
  }
  ~Mutex() {
    int ret = ::pthread_mutex_destroy(&mutex_);
    CHECK_EQ(ret, 0) << "pthread_mutex_destroy error: " << strerror(ret);
  }

  void Lock() {
    int ret = ::pthread_mutex_lock(&mutex_);
    CHECK_EQ(ret, 0) << "pthread_mutex_lock error: " << strerror(ret);
  }
  void UnLock() {
    int ret = ::pthread_mutex_unlock(&mutex_);
    CHECK_EQ(ret, 0) << "pthread_mutex_unlock error: " << strerror(ret);
  }

  pthread_mutex_t* mutex() const {
    return &mutex_;
  }

private:
  pthread_mutex_t mutex_;

  DISALLOW_COPY_AND_ASSIGN(Mutex);
};

class ScopedMutex {
  public:
    ScopedMutex(Mutex* mutex)
        : mutex_(mutex) {
      mutex_->Lock();
    }
    ~ScopedMutex() {
      mutex_->UnLock();
    }
  private:
    Mutex* mutex_;

    DISALLOW_COPY_AND_ASSIGN(ScopedMutex);
};

// CLOCK_MONOTONIC: // man clock_getres
//     Clock that cannot be set and represents monotonic
//     time since some unspecified starting point.  This
//     clock is not affected by discontinuous jumps in
//     the system  time  (e.g.,  if the system administrator
//     manually changes the clock), but is affected by the
//     incremental adjustments performed by adjtime(3) and NTP.
// CLOCK_MONOTONIC_RAW (since Linux 2.6.28; Linux-specific)
//     Similar to CLOCK_MONOTONIC, but provides access to a
//     raw hardware-based time that is not subject to NTP
//     adjustments or the incremental adjustments performed
//     by adjtime(3).
class SyncEvent {
 public:
  SyncEvent(bool manual_set = false, bool is_signaled = false)
      : manual_set_(manual_set),
        is_signaled_(is_signaled) {
    pthread_condattr_t attr;
    ::pthread_condattr_init(&attr);
    ::pthread_condattr_setclock(&attr, CLOCK_MONOTONIC_RAW);
    ::pthread_condattr_destroy(&attr);
    ::pthread_cond_init(&cond_, &attr);
  }
  ~SyncEvent() {
    ::pthread_cond_destroy(&cond_);
  }

  bool IsSignal() {
    ScopedMutex l(&mutex_);
    return is_signaled_;
  }
  void Reset() {
    CHECK(manual_set_);
    is_signaled_ = false;
  }

  void Wait();
  // return false iif timedout.
  bool TimeWait(uint32 micro_sec);

  void Signal();

 private:
  bool manual_set_;
  bool is_signaled_;

  pthread_cond_t cond_;
  Mutex mutex_;

  const static uint64 kNanoPerSec = 1000 * 1000 * 1000;

  DISALLOW_COPY_AND_ASSIGN(SyncEvent);
};
// TODO: rw_lock

#endif /* THREAD_UTIL_H_ */
