#include "timer_queue.h"
#include "event_manager.h"

namespace {

void handleTimerQueueEvent(int fd, void* arg, uint8 event,
                           TimeStamp time_stamp) {
  async::TimerQueue* q = static_cast<async::TimerQueue*>(arg);
  q->handleRead(time_stamp);
}
}

namespace async {

bool TimerQueue::Init() {
  if (event_ != NULL) return false;

  event_.reset(new Event);
  event_->fd = INVALID_FD;
  event_->arg = this;
  event_->event = EV_READ;
  event_->cb = handleTimerQueueEvent;

  return true;
}

void TimerQueue::runAt(Closure* cb, TimeStamp time_stamp) {
  if (ev_mgr_->inValidThread()) {
    runAtInternal(cb, time_stamp);
    return;
  }

  ev_mgr_->runInLoop(
      ::NewCallback(this, &TimerQueue::runAtInternal, cb, time_stamp));
}

void TimerQueue::runAtInternal(Closure* closure, TimeStamp time_stamp) {
  ev_mgr_->assertThreadSafe();

  TimeStamp expired = delegate_->insert(time_stamp, closure);
  if (!actived_ || expired < expired_time_) {
    reset(expired);
  }
}

void TimerQueue::handleRead(TimeStamp time_stamp) {
  actived_ = false;

  TimeStamp next_expired(time_stamp);
  if (delegate_->fireActivedTimer(time_stamp, &next_expired)) {
    reset(next_expired);
  }
}

}
