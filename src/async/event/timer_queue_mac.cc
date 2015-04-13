#include "timer_queue_mac.h"
#include "kqueue_impl.h"

#ifdef __APPLE__

namespace async {

const std::string TimerQueueMac::dummy_path_ = std::string("/dev/zero");

TimerQueueMac::TimerQueueMac(KqueueImpl* kq, Delegate* delegate)
    : TimerQueue(kq, delegate), kq_(kq), dummy_(INVALID_FD) {
}

bool TimerQueueMac::Init() {
  dummy_ = ::open(dummy_path_.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK);
  if (dummy_ == INVALID_FD) {
    PLOG(WARNING)<< "open error, path: " << dummy_path_;
    return false;
  }
  if (!TimerQueue::Init()) {
    closeWrapper(dummy_);
    return false;
  }

  event_->fd = dummy_;

  return true;
}

void TimerQueueMac::reset(const TimeStamp time_stamp) {
  expired_time_ = time_stamp;
  actived_ = true;

  kq_->setTimer(event_.get(), time_stamp.microSecs());
}
}

#endif
