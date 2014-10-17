#include "thread_util.h"

void SyncEvent::Signal() {
  ScopedMutex l(&mutex_);
  if (is_signaled_) return;

  is_signaled_ = true;
  ::pthread_cond_broadcast(&cond_);
}

void SyncEvent::Wait() {
  ScopedMutex l(&mutex_);
  if (is_signaled_) {
    if (!manual_set_) {
      is_signaled_ = false;
    }
    return;
  }

  ::pthread_cond_wait(&cond_, mutex_.mutex());
  CHECK(is_signaled_);
  if (!manual_set_) {
    is_signaled_ = false;
  }
}

bool SyncEvent::TimeWait(uint32 micro_sec) {
  ScopedMutex l(&mutex_);
  if (is_signaled_) {
    if (!manual_set_) {
      is_signaled_ = false;
    }
    return true;
  }

  timespec ts = { 0 };
  clock_getres(CLOCK_MONOTONIC_RAW, &ts);

  ts.tv_nsec += micro_sec * 1000 % kNanoPerSec;
  ts.tv_sec += micro_sec * 1000 / kNanoPerSec;
  int ret = ::pthread_cond_timedwait(&cond_, mutex_.mutex(), &ts);
  if (ret != 0) {
    DCHECK_EQ(ret, ETIMEDOUT)<< ret;
    CHECK(!is_signaled_);
    return false;
  }

  CHECK(is_signaled_);
  if (!manual_set_) {
    is_signaled_ = false;
  }
  return true;
}

