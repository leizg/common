#ifndef SPIN_LOCK_H_
#define SPIN_LOCK_H_

#include "macro_def.h"
#include "thread_util.h" // for microSleep

namespace detail {
class Waiter {
  public:
    explicit Waiter(uint32 interval = 100)
        : interval_(interval), guard_(0) {
    }

    void wait() {
      if (++guard_ > 3000) {
        microSleep(interval_, true /*ignore eintr*/);
      }
    }

  private:
    uint32 interval_;
    uint32 guard_;

    DISALLOW_COPY_AND_ASSIGN(Waiter);
};
}

class SpinLock {
  public:
    SpinLock()
        : lock_(0) {
    }
    virtual ~SpinLock() {
      DCHECK_EQ(lock_, 0);
    }

    void lock() {
      detail::Waiter waiter(100);
      while (__sync_lock_test_and_set(&lock_, 1) != 1) {
        waiter.wait();
      }
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
    explicit ScopedSpinlock(SpinLock* lock)
        : lock_(lock) {
      DCHECK_NOTNULL(lock);
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
