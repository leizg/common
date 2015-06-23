#pragma once

#include "base/base.h"

class ThreadSafe {
  public:
    virtual ~ThreadSafe() {
    }

    bool inValidThread() const {
      return pthread_self() == pid_;
    }

    void assertThreadSafe() const {
      CHECK(inValidThread());
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

