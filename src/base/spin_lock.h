#ifndef SPIN_LOCK_H_
#define SPIN_LOCK_H_

#include "macro_def.h"

class SpinLock {
  public:
    SpinLock()
        : lock_(0) {
    }
    virtual ~SpinLock() {
    }

    void lock() {
      while (__sync_lock_test_and_set(&lock_, 1) != 1)
        ;
    }
    void unLock() {
      __sync_lock_release(&lock_);
    }

  private:
    int lock_;

    DISALLOW_COPY_AND_ASSIGN(SpinLock);
};

class ScopedSpinlock {
  public:
    ScopedSpinlock(SpinLock* lock)
        : lock_(lock) {
      lock->lock();
    }
    ~ScopedSpinlock() {
      lock_->unLock();
    }

  private:
    SpinLock* lock_;

    DISALLOW_COPY_AND_ASSIGN(ScopedSpinlock);
};

#endif /* SPIN_LOCK_H_ */
