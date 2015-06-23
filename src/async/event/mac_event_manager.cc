#ifdef __APPALE__

#include "mac_event_manager.h"

namespace {

class KQueueTimer : public async::EventManager::TimerDelegate {
  public:
    KQueueTimer(async::KqueueImpl* kq)
        : kq_(kq) {
      DCHECK_NOTNULL(kq);
    }
    virtual ~KQueueTimer() {
    }

  private:
    async::KqueueImpl* kq_;

    // called by EventManager::Init.
    virtual bool Init();
    virtual void runAt(Closure* cb, const TimeStamp& ts);

    DISALLOW_COPY_AND_ASSIGN(KQueueTimer);
};

bool KQueueTimer::Init() {
  return true;
}

void KQueueTimer::runAt(Closure* cb, const TimeStamp& ts) {
  kq_->runAt(cb, ts);
}
}

namespace async {

KqueueImpl::~KqueueImpl() {
  Stop();
}

bool KqueueImpl::Init() {
  if (kp_fd_ != INVALID_FD) return true;

  kp_fd_ = ::kqueue();
  if (kp_fd_ == INVALID_FD) {
    PLOG(WARNING)<< "kqueue error";
    return false;
  }

  events_.resize(kTriggerNumber);
  timer_delegate_.reset(new KQueueTimer(this));
  if (!EventManager::Init() || !timer_delegate_->Init()) {
    ::close(kp_fd_);
    kp_fd_ = INVALID_FD;
    timer_delegate_.reset();
    return false;
  }

  return true;
}

void KqueueImpl::Loop(SyncEvent* start_event) {
  stop_ = false;
  if (!inValidThread()) {
    update();
    if (start_event != NULL) {
      start_event->Signal();
    }
  }

  timespec ts = { 1, 0 };
  while (!stop_) {
    int trigger_count = ::kevent(kp_fd_, NULL, 0, events_.data(),
                                 kTriggerNumber, &ts);
    if (trigger_count == -1) {
      if (errno != EINTR) {
        PLOG(WARNING)<< "kevent error";
      }
      continue;
    }
    TimeStamp time_stamp(Now());
    for (uint32 i = 0; i < static_cast<uint32>(trigger_count); ++i) {
      const struct kevent& kevent = events_[i];
      uint32 flags = 0;
      if ((kevent.fflags & EVFILT_READ) || (kevent.fflags & EVFILT_TIMER)) {
        flags |= EV_READ;
      }
      if (kevent.fflags & EVFILT_WRITE) flags |= EV_WRITE;

      Event* ev = static_cast<Event*>(kevent.udata);
      ev->cb(ev->fd, ev->arg, flags, time_stamp);

      if (kevent.fflags & EVFILT_TIMER) {
        ev_map_.erase(ev->fd);
      }
    }
  }
}

bool KqueueImpl::LoopInAnotherThread() {
  SyncEvent sync;
  loop_pthread_.reset(
      new StoppableThread(
          NewPermanentCallback(this, &KqueueImpl::Loop, &sync)));
  if (loop_pthread_->Start()) {
    if (!sync.TimeWait(3 * 1000)) return false;
    return true;
  }

  return false;
}

void KqueueImpl::Stop() {
  EventManager::Stop();

  if (loop_pthread_ != NULL) {
    loop_pthread_->Join();
    loop_pthread_.reset();
  }

  if (kp_fd_ != INVALID_FD) {
    ::close(kp_fd_);
    kp_fd_ = INVALID_FD;
  }
  stop_ = true;
}

bool KqueueImpl::Add(Event* ev) {
  if (ev_map_.count(ev->fd) != 0) {
    LOG(WARNING)<< "kevent already registe, fd: " << ev->fd;
    return false;
  }

  uint32 flags = 0;
  if (ev->event & EV_READ) flags |= EVFILT_READ;
  if (ev->event & EV_WRITE) flags |= EVFILT_WRITE;
  struct kevent event = {0};
  EV_SET(&event, ev->fd, flags, EV_ADD, 0, 0, ev);
  int ret = ::kevent(kp_fd_, &event, 1, NULL, 0, NULL);
  if (ret != 0) {
    PLOG(WARNING) << "kevent error";
    return false;
  }

  ev_map_[ev->fd] = ev;
  return true;
}

void KqueueImpl::Mod(Event* ev) {
  DCHECK_EQ(ev_map_.count(ev->fd), 1);

  uint32 flags = 0;
  if (ev->event & EV_READ) flags |= EVFILT_READ;
  if (ev->event & EV_WRITE) flags |= EVFILT_WRITE;
  struct kevent event = { 0 };
  // Re-adding an existing event will modify the parameters of
  // the original event, and not result in a duplicate entry.
  EV_SET(&event, ev->fd, flags, EV_ADD, 0, 0, ev);
  int ret = ::kevent(kp_fd_, &event, 1, NULL, 0, NULL);
  if (ret != 0) {
    PLOG(WARNING)<< "kevent error";
    // maybe should delete directly.
  }
}

void KqueueImpl::Del(const Event& ev) {
  DCHECK_EQ(ev_map_.count(ev.fd), 1);

  uint32 flags = 0;
  if (ev.event & EV_READ) flags |= EVFILT_READ;
  if (ev.event & EV_WRITE) flags |= EVFILT_WRITE;
  struct kevent event = { 0 };
  EV_SET(&event, ev.fd, flags, EV_DELETE, 0, 0, NULL);
  int ret = ::kevent(kp_fd_, &event, 1, NULL, 0, NULL);
  if (ret != 0) {
    PLOG(WARNING)<<"kevent error";
  }

  ev_map_.erase(ev.fd);
}

void KqueueImpl::setTimer(Event* ev, uint32 exipred) {
  if (ev_map_.count(ev->fd) != 1) {
    ev_map_[ev->fd] = ev;
  }

  struct kevent event = { 0 };
  EV_SET(&event, ev->fd, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, exipred, ev);
  int ret = ::kevent(kp_fd_, &event, 1, NULL, 0, NULL);
  if (ret != 0) {
    PLOG(WARNING)<<"kevent error";
  }
}
}
#endif
