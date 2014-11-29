#include "closure_pump.h"

#if __linux__
#include <sys/timerfd.h>
#endif

namespace {

void handleSignal(int fd, void* arg, uint8 revent,
                  const TimeStamp& time_stamp) {
  io::detail::EventPipe* p = static_cast<io::detail::EventPipe*>(arg);
  p->handlePipeRead();
}

#if __linux__
void handleTimerQueueEvent(int fd, void* arg, int event,
    const TimeStamp& time_stamp) {
  io::detail::TimerQueue* q = static_cast<io::detail::TimerQueue*>(arg);
  q->handleRead(time_stamp);
}

// TODO: strgary.
class TimerListImpl : public io::detail::TimerQueue::Delegate {
  public:
  TimerListImpl() {
  }
  virtual ~TimerListImpl() {
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

  DISALLOW_COPY_AND_ASSIGN(TimerListImpl);
};

TimeStamp TimerListImpl::insert(const TimeStamp time_stamp, Closure* cb) {
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

bool TimerListImpl::fireActivedTimer(const TimeStamp& time_stamp,
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
#endif
}

namespace io {

detail::EventPipe::~EventPipe() {
if (event_fd_[0] != INVALID_FD) {
  ev_mgr_->Del(event_);
  ::close(event_fd_[0]);
}

if (event_fd_[1] != INVALID_FD) {
  ::close(event_fd_[1]);
}
}

bool detail::EventPipe::Init() {
int ret;
#if __linux__
ret = ::pipe2(event_fd_, O_NONBLOCK | FD_CLOEXEC);
#else
ret = ::pipe(event_fd_);
#endif
if (ret != 0) {
  PLOG(WARNING)<< "pipe error";
  return false;
}

#if not __linux__
setFdCloExec(event_fd_[0]);
setFdCloExec(event_fd_[1]);
setFdNonBlock(event_fd_[0]);
setFdNonBlock(event_fd_[1]);
#endif
event_.fd = event_fd_[0];
event_.event = EV_READ;
event_.arg = this;
event_.cb = handleSignal;

return ev_mgr_->Add(&event_);
}

void detail::EventPipe::runInLoop(Closure* cb) {
if (ev_mgr_->inValidThread()) {
  cb->Run();
  return;
}

cb_queue_.push_back(cb);
uint8 c;
::send(event_fd_[1], &c, sizeof(c), 0);
}

void detail::EventPipe::handlePipeRead() {
uint64 c;
::recv(event_fd_[0], &c, sizeof(c), 0);

std::deque<Closure*> cbs;
{
  ScopedMutex l(&mutex_);
  cbs.swap(cb_queue_);
}

for (uint32 i = 0; i < cbs.size(); ++i) {
  Closure* cb = cbs[i];
  cb->Run();
}
}
#if __linux__
detail::TimerQueue::~TimerQueue() {
if (timer_fd_ != INVALID_FD) {
  ev_mgr_->Del(*event_);
  ::close(timer_fd_);
}
}

bool detail::TimerQueue::Init() {
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
event_->cb = handleTimerQueueEvent;

if (!ev_mgr_->Add(event_.get())) {
  ::close(timer_fd_);
  timer_fd_ = INVALID_FD;
  event_.reset();
}

delegate_.reset(new TimerListImpl);
return true;
}

void detail::TimerQueue::runAt(Closure* closure, const TimeStamp& time_stamp) {
if (ev_mgr_->inValidThread()) {
  runAtInternal(closure, time_stamp);
  return;
}

ev_mgr_->runInLoop(
    NewCallback(this, &TimerQueue::runAtInternal, closure, time_stamp));
}

void detail::TimerQueue::runAtInternal(Closure* closure,
  const TimeStamp time_stamp) {
ev_mgr_->assertThreadSafe();

TimeStamp expired = delegate_->insert(time_stamp, closure);
if (!actived_ || expired < expired_time_) {
  reset(expired);
}
}

void detail::TimerQueue::reset(const TimeStamp time_stamp) {
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

void detail::TimerQueue::handleRead(const TimeStamp& time_stamp) {
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
#endif

bool ClosurePump::Init() {
ev_pipe_.reset(new detail::EventPipe(ev_mgr_));
if (!ev_pipe_->Init()) return false;

#if __linux__
timer_queue_.reset(new detail::TimerQueue(ev_mgr_));
if (!timer_queue_->Init()) {
  ev_pipe_.reset();
  return false;
}
#endif

return true;
}
}
