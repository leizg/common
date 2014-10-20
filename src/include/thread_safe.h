#ifndef THREAD_SAFE_H_
#define THREAD_SAFE_H_

#include "base/base.h"

#ifndef DEBUG {

class ThreadSafe {
  public:
    virtual ~ThreadSafe() {
    }

    bool inValidThread() const {
      return pthread_self() == pid_;
    }

  protected:
    ThreadSafe()
        : pid_(::pthread_self()) {
    }

  private:
    pthread_t pid_;

    DISALLOW_COPY_AND_ASSIGN(ThreadSafe);
};
#else

class ThreadSafe {
  public:
  ThreadSafe() {}

  bool inValidThread() const {
    return true;
  }

  private:
  DISALLOW_COPY_AND_ASSIGN(ThreadSafe);
};

#endif

#endif /* THREAD_SAFE_H_ */
