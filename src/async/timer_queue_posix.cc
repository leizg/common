#include "timer_queue_posix.h"
#include "event_manager.h"

#if __linux__
#include <sys/timerfd.h>

namespace {

bool createTimerFd(int* fd) {
  /*
   * The clockid argument specifies the clock that is used to
   * mark the progress of the timer, and must be either
   * CLOCK_REALTIME or CLOCK_MONOTONIC.  CLOCK_REALTIME is a
   * settable system-wide clock.  CLOCK_MONOTONIC is a nonsettable
   * clock that is not affected by  discontinuous  changes  in  the
   * system clock (e.g., manual changes to system time).
   */
  int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer_fd == -1) {
    PLOG(WARNING)<< "timerfd_create error";
    return false;
  }

  *fd = timer_fd;
  return true;
}
}

namespace async {

TimerQueuePosix::~TimerQueuePosix() {

}

bool TimerQueuePosix::init() {
  if (timer_fd_ != INVALID_FD) return false;
  if (!createTimerFd(&timer_fd_)) return false;

  TimerQueue::init();
  event_->fd = timer_fd_;
  if (!ev_mgr_->Add(event_.get())) {
    closeWrapper(timer_fd_);
    event_.reset();
    return false;
  }

  return true;
}

void TimerQueuePosix::destory() {
  TimerQueue::destory();

  closeWrapper(timer_fd_);
}

void TimerQueuePosix::reset(TimeStamp time_stamp) {
  itimerspec ts;
  ::memset(&ts, 0, sizeof(ts));
  ts.it_value = time_stamp.toTimeSpec();

  int ret = ::timerfd_settime(timer_fd_, 0, &ts, NULL);
  if (ret != 0) {
    PLOG(WARNING)<<"timerfd_settime error";
    return;
  }

  expired_time_ = time_stamp;
  actived_ = true;
}

void TimerQueuePosix::clearData() {
  uint64 count = 0;

  while (true) {
    int ret = ::read(timer_fd_, &count, sizeof(count));
    if (ret == 0) return;
    else if (ret == -1) {
      switch (errno) {
        case EINTR:
          continue;
        case EWOULDBLOCK:
          return;
        default:
          break;
      }

      DLOG(WARNING)<<"read timerfd error, fd:" << timer_fd_;
      return;
    }
  }
}
}
#endif
