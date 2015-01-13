#include "timer_queue.h"

namespace {

void handleTimerQueueEvent(int fd, void* arg, uint8 event,
                           const TimeStamp& time_stamp) {
  async::TimerQueue* q = static_cast<async::TimerQueue*>(arg);
  q->handleRead(time_stamp);
}
}

namespace async {

bool TimerQueue::Init() {
  event_.reset(new Event);
  event_->fd = INVALID_FD;
  event_->arg = this;
  event_->event = EV_READ;
  event_->cb = handleTimerQueueEvent;

  return true;
}

void TimerQueue::runAt(Closure* closure, const TimeStamp& time_stamp) {
  if (ev_mgr_->inValidThread()) {
    runAtInternal(closure, time_stamp);
    return;
  }

  ev_mgr_->runInLoop(
      ::NewCallback(this, &TimerQueue::runAtInternal, closure, time_stamp));
}

void TimerQueue::runAtInternal(Closure* closure, const TimeStamp time_stamp) {
  ev_mgr_->assertThreadSafe();

  TimeStamp expired = delegate_->insert(time_stamp, closure);
  if (!actived_ || expired < expired_time_) {
    reset(expired);
  }
}

void TimerQueue::handleRead(const TimeStamp& time_stamp) {
  actived_ = false;

  TimeStamp next_expired(time_stamp);
  if (delegate_->fireActivedTimer(time_stamp, &next_expired)) {
    reset(next_expired);
  }
}

}
