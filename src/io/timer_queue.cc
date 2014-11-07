#include "timer_queue.h"
#include "event_manager.h"

#include <sys/timerfd.h>

namespace {

void HandleEvent(int fd, void* arg, int event, const TimeStamp& time_stamp) {
  io::TimerQueue* q = static_cast<io::TimerQueue*>(arg);
  q->handleRead(time_stamp);
}

class TimerList : public io::TimerQueue::Delegate {
  public:
    TimerList() {
    }
    virtual ~TimerList() {
      for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        const Entry& entry = *it;
        delete entry.second;
      }
      entries_.clear();
    }

  private:
    typedef std::pair<TimeStamp, Closure*> Entry;
    typedef std::list<Entry> EntryList;
    EntryList entries_;

    virtual TimeStamp insert(const TimeStamp time_stamp, Closure* cb);
    virtual bool fireActivedTimer(const TimeStamp& time_stamp,
                                  TimeStamp* next_expired);

    DISALLOW_COPY_AND_ASSIGN(TimerList);
};

TimeStamp TimerList::insert(const TimeStamp time_stamp, Closure* cb) {
  EntryList::iterator pos;
  for (pos = entries_.begin(); pos != entries_.end(); ++pos) {
    const TimeStamp& ts = (*pos).first;
    if (ts >= time_stamp) {
      break;
    }
  }

  entries_.insert(pos, std::make_pair(time_stamp, cb));
  DCHECK(!entries_.empty());
  return entries_.begin()->first;
}

bool TimerList::fireActivedTimer(const TimeStamp& time_stamp,
                                 TimeStamp* next_expired) {
  while (!entries_.empty()) {
    const Entry& entry = *entries_.begin();
    if (entry.first > time_stamp) {
      break;
    }

    Closure* cb = entry.second;
    AutoRunner<Closure> r(cb);
    entries_.pop_front();
  }

  if (!entries_.empty()) {
    *next_expired = entries_.begin()->first;
    return true;
  }
  return false;
}
}

namespace io {

TimerQueue::~TimerQueue() {
  if (timer_fd_ != INVALID_FD) {
    ev_mgr_->Del(*event_);
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

void TimerQueue::runAt(Closure* closure, const TimeStamp& time_stamp) {
  if (ev_mgr_->inValidThread()) {
    runAtInternal(closure, time_stamp);
    return;
  }

  ev_mgr_->runInLoop(
      NewCallback(this, &TimerQueue::runAtInternal, closure, time_stamp));
}

void TimerQueue::runAtInternal(Closure* closure, const TimeStamp time_stamp) {
  ev_mgr_->assertThreadSafe();

  TimeStamp expired = delegate_->insert(time_stamp, closure);
  if (!actived_ || expired < expired_time_) {
    reset(expired);
  }
}

void TimerQueue::reset(const TimeStamp time_stamp) {
  itimerspec ts;
  ::memset(&ts, 0, sizeof(ts));
  ts.it_value.tv_sec = time_stamp.timeSpec();

  int ret = ::timerfd_settime(timer_fd_, 0, &ts, NULL);
  if (ret != 0) {
    PLOG(WARNING)<<"timerfd_settime error";
    return;
  }

  expired_time_ = time_stamp;
  actived_ = true;
}

void TimerQueue::handleRead(const TimeStamp& time_stamp) {
  uint64 count = 0;
  while (true) {
    int ret = ::read(timer_fd_, &count, sizeof(count));
    if (ret == 0) return;
    else if (ret == -1) {
      if (errno == EINTR) continue;
      else if (errno == EWOULDBLOCK || errno == EAGAIN) break;
      DLOG(WARNING)<< "read timerfd error, fd:" << timer_fd_;
    }
  }

  actived_ = false;
  TimeStamp next_expired(time_stamp);
  if (delegate_->fireActivedTimer(time_stamp, &next_expired)) {
    reset(next_expired);
  }
}
}
