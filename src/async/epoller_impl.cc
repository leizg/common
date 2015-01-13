#include "epoller_impl.h"

#ifdef __linux__

#include "timer_list.h"
#include "timer_queue_posix.h"

namespace async {

EpollerImpl::~EpollerImpl() {
  Stop();
  closeWrapper(ep_fd_);
}

bool EpollerImpl::Init() {
  if (ep_fd_ != INVALID_FD) return false;

  /*
   * In the initial epoll_create() implementation, the size argument
   * informed the kernel of the number of file descriptors that the
   * caller expected to add to the epoll instance.  The kernel used
   * this information  as  a  hint  for  the  amount of space to
   * initially allocate in internal data structures describing events.
   * (If necessary, the kernel would allocate more space if the caller's
   * usage exceeded the hint given in size.)  Nowadays, this hint is
   * no longer required (the kernel dynamically sizes the required data
   * structures without needing the hint), but size must still  be
   * greater than zero, in order to ensure backward compatibility
   * when new epoll applications are run on older kernels.
   */
  ep_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
  if (ep_fd_ == INVALID_FD) {
    PLOG(WARNING)<< "epoll_create error";
    return false;
  }

  setFdNonBlock(ep_fd_);

  if (!EventManager::Init()) {
    closeWrapper(ep_fd_);
  }
  timer_delegate_.reset(new TimerQueuePosix(this, new TimerListImpl));
  if (!timer_delegate_->Init()) {
    DLOG(INFO)<< "timer delegate init error";
    closeWrapper(ep_fd_);
    thread_safe_delegate_.reset();
    return false;
  }

  epoll_event ev;
  ::memset(&ev, 0, sizeof(ev));
  events_.resize(kTriggerNumber, ev);
  return true;
}

void EpollerImpl::Loop(SyncEvent* start_event) {
  stop_ = false;
  DCHECK_NE(ep_fd_, INVALID_FD);
  if (!inValidThread()) update();
  if (start_event != NULL) start_event->Signal();

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

    TimeStamp time_stamp = Now();
    DCHECK_LE(trigger_number, kTriggerNumber);
    for (uint32 i = 0; i < trigger_number; ++i) {
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

bool EpollerImpl::LoopInAnotherThread() {
  SyncEvent start_event(false, false);
  loop_pthread_.reset(
      new StoppableThread(
          ::NewPermanentCallback(this, &EpollerImpl::Loop, &start_event)));
  if (loop_pthread_->Start()) {
    if (!start_event.TimeWait(3 * 1000)) {
      loop_pthread_.reset();
      LOG(WARNING)<< "event loop thread start error";
      return false;
    }
    DCHECK(!inValidThread());
    return true;
  }

  loop_pthread_.reset();
  return false;
}

void EpollerImpl::Stop() {
  if (stop_) return;

  stop_ = true;
  loop_pthread_.reset();
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

bool EpollerImpl::Add(Event* ev) {
  CHECK_NOTNULL(ev);
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

void EpollerImpl::Mod(Event* ev) {
  CHECK_NOTNULL(ev);
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

void EpollerImpl::Del(const Event& ev) {
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
