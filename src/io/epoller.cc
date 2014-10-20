#include "epoller.h"
#include <sys/epoll.h>

namespace io {

Epoller::~Epoller() {
  if (ep_fd_ != INVALID_FD) {
    ::close(ep_fd_);
    ep_fd_ = INVALID_FD;
  }
}

void Epoller::Init() {
  if (ep_fd_ != INVALID_FD) return;

  /*
   * In the initial epoll_create() implementation, the size argument
   * informed the kernel of the number of file descriptors that the
   * caller expected to add to the epoll instance.  The kernel used
   * this information  as  a  hint  for  the  amount of space to
   * initially allocate in internal data structures describing events.
   *  (If necessary, the kernel would allocate more space if the caller's
   *   usage exceeded the hint given in size.)  Nowadays, this hint is
   *   no longer required (the kernel dynamically sizes the required data
   *   structures without needing the hint), but size must still  be
   *   greater than zero, in order to ensure backward compatibility
   *   when new epoll applications are run on older kernels.
   */
  ep_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
  if (ep_fd_ == INVALID_FD) {
    PLOG(WARNING)<< "epoll_create error";
    return;
  }

  SetFdNonBlock(ep_fd_);

  epoll_event ev;
  ::memset(&ev, 0, sizeof(ev));
  events_.resize(kTriggerNumber, ev);

  if (!InitPipe()) {
    ::close(ep_fd_);
    ep_fd_ = INVALID_FD;
  }
}

void Epoller::Loop() {
  if (!inValidThread()) {
    update();
    start_event_.Signal();
  }

  stop_ = false;
  CHECK_NE(ep_fd_, INVALID_FD);

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

bool Epoller::LoopInAnotherThread() {
  loop_pthread_.reset(new StoppableThread(NewCallback(this, &Epoller::Loop)));
  if (!loop_pthread_->Start()) return false;
  start_event_.Wait();
  CHECK(!inValidThread());
  return true;
}

void Epoller::Stop() {
  if (stop_) return;

  stop_ = true;
  loop_pthread_.reset();
}

uint32 Epoller::ConvertEvent(uint8 event) {
  uint32 flags = 0;
  if (event & EV_READ) {
    flags |= EPOLLIN;
  }
  if (event & EV_WRITE) {
    flags |= EPOLLOUT;
  }

  return flags;
}

bool Epoller::Add(Event* ev) {
  CHECK_NOTNULL(ev);
  EvMap::iterator it = ev_map_.find(ev->fd);
  if (it != ev_map_.end()) {
    LOG(WARNING)<< "already registe fd: " << ev->fd;
    return false;
  }

  epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = ConvertEvent(ev->event);
  event.data.ptr = ev;

  int ret = ::epoll_ctl(ep_fd_, EPOLL_CTL_ADD, ev->fd, &event);
  if (ret != 0) {
    PLOG(WARNING)<< "epoll_ctl error";
    return false;
  }
  ev_map_[ev->fd] = ev;
  return true;
}

void Epoller::Mod(Event* ev) {
  CHECK_NOTNULL(ev);
  EvMap::iterator it = ev_map_.find(ev->fd);
  if (it == ev_map_.end()) {
    LOG(WARNING)<< "not registe fd: " << ev->fd;
    return;
  }

  epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = ConvertEvent(ev->event);
  event.data.ptr = ev;

  int ret = ::epoll_ctl(ep_fd_, EPOLL_CTL_MOD, ev->fd, &event);
  PLOG_IF(WARNING, ret != 0) << "epoll_ctl error";
}

void Epoller::Del(const Event& ev) {
  EvMap::iterator it = ev_map_.find(ev.fd);
  if (it == ev_map_.end()) {
    DLOG(WARNING)<< "not registe fd: " << ev.fd;
    return;
  }
  ev_map_.erase(it);

  epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = ConvertEvent(ev.event);
  event.data.ptr = ev;

  int ret = ::epoll_ctl(ep_fd_, EPOLL_CTL_DEL, ev.fd, &event);
  PLOG_IF(WARNING, ret != 0) << "EPOLL_CTL_DEL error";
}

}
