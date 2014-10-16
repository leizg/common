#include "timer_queue.h"
#include "event_manager.h"

#include <sys/timerfd.h>

namespace {
using namespace io;

void HandleEvent(int fd, void* arg, int event, const TimeStamp& time_stamp) {
  TimerQueue* q = static_cast<TimerQueue*>(arg);
  q->handleRead(time_stamp);
}
}

namespace io {

TimerQueue::~TimerQueue() {
  if (timer_fd_ != INVALID_FD) {
    ev_mgr_->Del((const Event&) *event_);
    ::close(timer_fd_);
  }
}

bool TimerQueue::Init() {
  if (timer_fd_ != INVALID_FD) return true;
  /*
   * The clockid argument specifies the clock that is used to
   * mark the progress of the timer, and must be either
   * CLOCK_REALTIME or CLOCK_MONOTONIC.  CLOCK_REALTIME is a
   * settable system-wide clock.  CLOCK_MONOTONIC is a nonsettable
   * clock that is not affected by  discontinuous  changes  in  the
   *  system clock (e.g., manual changes to system time).
   */
  timer_fd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer_fd_ == -1) {
    PLOG(WARNING)<< "timerfd_create error";
    return false;
  }

  event_.reset(new Event);
  event_->fd = timer_fd_;
  event_->arg = this;
  event_->event = EV_READ;
  event_->cb = HandleEvent;

  if (!ev_mgr_->Add(event_.get())) {
    ::close(timer_fd_);
    timer_fd_ = INVALID_FD;
    event_.reset();
  }

  return true;
}

TimerId TimerQueue::AddTimer(Closure* closure, uint32 interval, bool repeated) {
  Timer* timer = new Timer(repeated, interval, closure);
  TimerId timer_id;
  timer_id.timer = timer;
  timer_id.timer_id = timer->id();

  // FIXME: run in loop thread.
  id_set_.insert(timer->id());
  if (insert(timer)) {
    Reset();
  }

  return timer_id;
}

void TimerQueue::CancelTimer(const TimerId& timer_id) {
  // FIXME: run in loop thread.
  if (id_set_.count(timer_id.timer_id) == 0) return;

  id_set_.erase(timer_id.timer_id);
  Timer* timer = timer_id.timer;

  EntryList::iterator it;
  for (it = entries_.begin(); it != entries_.end(); ++it) {
    const Entry& item = *it;
    if (item.second->id() == timer_id.timer_id) {
      break;
    }
  }

  CHECK_NE(it, entries_.end());
  bool first_changged = it == entries_.begin();
  entries_.erase(it);
  if (first_changged) Reset();
}

void TimerQueue::ReleaseActiveTimer(uint32 trigger_count,
                                    std::vector<Timer*>* timer_vec) {
  for (uint64 i = 0; i < trigger_count; ++i) {
    if (entries_.empty()) return;
    Timer* timer = entries_.begin()->second;
    timer->Run();
    timer_vec->push_back(timer);

    entries_.pop_front();
  }
}

bool TimerQueue::insert(Timer* timer) {
  Entry entry;
  entry.first = timer->ExpiredTime();
  entry.second = timer;

  EntryList::iterator it;
  for (it = entries_.begin(); it != entries_.end(); ++it) {
    const Entry& item = *it;
    if (item > entry) break;
  }
  bool first_changed = it == entries_.begin();
  entries_.insert(it, entry);

  return first_changed;
}

void TimerQueue::Reset() {
  itimerspec ts;
  ::memset(&ts, 0, sizeof(ts));

  if (!entries_.empty()) {
    const TimeStamp& time_stamp = entries_.begin()->second->ExpiredTime();
    ts.it_value.tv_sec = time_stamp.TimeSpec();
  }

  int ret = ::timerfd_settime(timer_fd_, 0, &ts, NULL);
  if (ret != 0) {
    PLOG(WARNING)<< "timerfd_settime error";
  }
}

void TimerQueue::handleRead(const TimeStamp& time_stamp) {
  uint64 trigger_count = 0;
  while (true) {
    int ret = ::read(timer_fd_, &trigger_count, sizeof(trigger_count));
    if (ret == -1) {
      if (errno == EINTR) continue;
      DLOG(WARNING)<< "read timerfd error, fd:" << timer_fd_;
      return;
    }
  }

  bool need_adjust = false;
  std::vector<Timer*> timer_vec;
  ReleaseActiveTimer(trigger_count, &timer_vec);
  for (std::vector<Timer*>::iterator it = timer_vec.begin();
      it != timer_vec.end(); ++it) {
    Timer* timer = *it;
    if (!timer->IsRepeated()) {
      id_set_.erase(timer->id());
      delete timer;
      continue;
    }

    timer->Reset();
    need_adjust |= insert(timer);
  }

  if (need_adjust) Reset();
}

}
