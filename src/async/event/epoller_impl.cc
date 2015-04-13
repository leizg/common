#include "epoller_impl.h"
#include "closure_proxy.h"

#ifdef __linux__

namespace {

bool createEventFd(int* epfd) {
  int fd = ::epoll_create1(EPOLL_CLOEXEC);
  if (fd == INVALID_FD) {
    PLOG(WARNING)<< "epoll_create error";
    return false;
  }

  setFdNonBlock(fd);

  *epfd = fd;
  return true;
}
}

namespace async {

bool EpollerImpl::init() {
  if (ep_fd_ != INVALID_FD) return false;
  if (!createEventFd(&ep_fd_)) return false;

  cb_delegate_.reset(new ClosureProxy(this));
  if (!cb_delegate_->init()) {
    closeWrapper(ep_fd_);
    cb_delegate_.reset();
    LOG(WARNING)<< "callback delegate init error";
    return false;
  }

  if (!EventManager::init()) {
    closeWrapper(ep_fd_);
    return false;
  }

  epoll_event ev;
  ::memset(&ev, 0, sizeof ev);
  events_.resize(kTriggerNumber, ev);

  return true;
}

void EpollerImpl::loop(SyncEvent* start_event) {
  stop_ = false;
  DCHECK_NE(ep_fd_, INVALID_FD);
  if (!inValidThread()) update();
  if (start_event != nullptr) start_event->Signal();

  DLOG(INFO)<< "start event loop...";
  while (!stop_) {
    int32 trigger_number = ::epoll_wait(ep_fd_, events_.data(), events_.size(),
                                        1);
    if (trigger_number == -1) {
      if (errno != EINTR) {
        PLOG(WARNING)<< "epoll_wait error";
      }
      continue;
    }

    TimeStamp time_stamp = TimeStamp::now();
    DCHECK_LE(trigger_number, kTriggerNumber);
    for (int32 i = 0; i < trigger_number; ++i) {
      Event* event = static_cast<Event*>(events_[i].data.ptr);

      uint8 flags = 0;
      if (events_[i].events & EPOLLIN) {
        flags |= EV_READ;
      }
      if (events_[i].events & EPOLLOUT) {
        flags |= EV_WRITE;
      }
      event->cb(event->fd, event->arg, flags, time_stamp);
    }
  }
}

bool EpollerImpl::loopInAnotherThread() {
  SyncEvent start_event(false, false);
  loop_pthread_.reset(
      new StoppableThread(
          ::NewPermanentCallback(this, &EpollerImpl::loop, &start_event)));
  if (loop_pthread_->Start()) {
    loop_pthread_.reset();
    return false;
  }

  if (!start_event.TimeWait(3 * TimeStamp::kMicroSecsPerMilliSecond)) {
    loop_pthread_.reset();
    LOG(WARNING)<< "event loop thread start error";
    return false;
  }
  DCHECK(!inValidThread());
  return true;
}

void EpollerImpl::stop(SyncEvent* ev) {
  if (stop_) {
    if (ev != nullptr) ev->Signal();
    return;
  }

  if (inValidThread()) {
    stopInternal(ev);
  } else {
    runInLoop(NewCallback(this, &EpollerImpl::stopInternal, ev));
  }
}

void EpollerImpl::stopInternal(SyncEvent* ev) {
  stop_ = true;
  EventManager::stop(ev);

  if (cb_delegate_ != nullptr) {
    cb_delegate_->destory();
    cb_delegate_.reset();
  }

  if (loop_pthread_ != nullptr) {
    loop_pthread_->Join();
    loop_pthread_.reset();
  }
  if (ev != nullptr) ev->Signal();
}

uint32 EpollerImpl::convertEvent(uint8 event) {
  uint32 flags = 0;
  if (event & EV_READ) {
    flags |= EPOLLIN;
  }
  if (event & EV_WRITE) {
    flags |= EPOLLOUT;
  }

  return flags;
}

bool EpollerImpl::add(Event* ev) {
  CHECK_NOTNULL(ev);
  assertThreadSafe();
  EvMap::iterator it = ev_map_.find(ev->fd);
  if (it != ev_map_.end()) {
    LOG(WARNING)<< "already registe fd: " << ev->fd;
    return false;
  }

  epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = convertEvent(ev->event);
  event.data.ptr = ev;

  int ret = ::epoll_ctl(ep_fd_, EPOLL_CTL_ADD, ev->fd, &event);
  if (ret != 0) {
    PLOG(WARNING)<< "epoll_ctl error";
    return false;
  }
  ev_map_[ev->fd] = ev;
  return true;
}

void EpollerImpl::mod(Event* ev) {
  CHECK_NOTNULL(ev);
  assertThreadSafe();
  EvMap::iterator it = ev_map_.find(ev->fd);
  if (it == ev_map_.end()) {
    LOG(WARNING)<< "not registe fd: " << ev->fd;
    return;
  }

  epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = convertEvent(ev->event);
  event.data.ptr = ev;

  int ret = ::epoll_ctl(ep_fd_, EPOLL_CTL_MOD, ev->fd, &event);
  PLOG_IF(WARNING, ret != 0) << "epoll_ctl error";
}

void EpollerImpl::del(const Event& ev) {
  assertThreadSafe();
  EvMap::iterator it = ev_map_.find(ev.fd);
  if (it == ev_map_.end()) {
    DLOG(WARNING)<< "not registe fd: " << ev.fd;
    return;
  }
  ev_map_.erase(it);

  epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = convertEvent(ev.event);
  event.data.ptr = NULL;

  int ret = ::epoll_ctl(ep_fd_, EPOLL_CTL_DEL, ev.fd, &event);
  PLOG_IF(WARNING, ret != 0) << "EPOLL_CTL_DEL error";
}

}
#endif
