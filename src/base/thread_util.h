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

// TODO: rw_lock and SyncEvent

#endif /* THREAD_UTIL_H_ */
