#include "thread_util.h"
#include <sys/time.h>

void microSleep(uint64 micro_secs, bool ignore_eintr) {
  while (true) {
    TimeStamp before = Now();
    timespec req = timespecFromMicrosecs(micro_secs);
    int ret = nanosleep(&req, NULL);
    switch (ret) {
      case -1:
        if (ignore_eintr && errno == EINTR) {
          continue;
        }  // fall though.
      default:
        break;
    }
    micro_secs -= TimeDiff(Now(), before);
  }
}

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

  int ret;
  timespec ts = { 0 };
#if 0
#ifndef __linux__
  ret = clock_getres(CLOCK_MONOTONIC, &ts);
#endif
#endif
  timeval tv;
  ret = ::gettimeofday(&tv, NULL);
  ts.tv_sec = tv.tv_sec;
  ts.tv_nsec = tv.tv_usec * 1000;
  DCHECK_EQ(ret, 0)<< ret;

  ts.tv_nsec += micro_sec * 1000 * 1000ULL;
  ts.tv_sec += ts.tv_nsec / kNanosecsPerSecond;
  ts.tv_nsec %= kNanosecsPerSecond;
  ret = ::pthread_cond_timedwait(&cond_, mutex_.mutex(), &ts);
  if (ret != 0) {
    DCHECK_EQ(ret, ETIMEDOUT)<< ret;
    DCHECK(!is_signaled_);
    return false;
  }

  CHECK(is_signaled_);
  if (!manual_set_) {
    is_signaled_ = false;
  }
  return true;
}
