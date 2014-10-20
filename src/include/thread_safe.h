#ifndef THREAD_SAFE_H_
#define THREAD_SAFE_H_

#include "base/base.h"

class ThreadSafe {
  public:
    virtual ~ThreadSafe() {
    }

    bool inValidThread() const {
      return pthread_self() == pid_;
    }

  protected:
    ThreadSafe() {
      update();
    }

    void update() {
      pid_ = ::pthread_self();
    }

  private:
    pthread_t pid_;

    DISALLOW_COPY_AND_ASSIGN(ThreadSafe);
};

#endif /* THREAD_SAFE_H_ */
