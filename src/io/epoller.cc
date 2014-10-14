#include "epoller.h"
#include <sys/epoll.h>

namespace io {

Epoller::~Epoller() {
  if (ep_fd_ != INVALID_FD) {
    ::close(ep_fd_);
  }
}

bool Epoller::Init() {
  if (ep_fd_ != INVALID_FD) return false;
  ep_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
  if (ep_fd_ == INVALID_FD) {
    PLOG(WARNING)<< "epoll_create error";
    return false;
  }

  SetFdNonBlock(ep_fd_);

  epoll_event ev;
  ::memset(&ev, 0, sizeof(ev));
  events_.resize(kTriggerNumber, ev);

  return InitPipe();
}

void Epoller::Loop() {
  if (!inLoopThread()) {
    loop_tid_ = ::pthread_self();
    // FIXME: notify by SyncEvent.
  }

  stop_ = false;
  CHECK_NE(ep_fd_, INVALID_FD);

  while (!stop_) {
    int32 trigger_number = ::epoll_wait(ep_fd_, events_.data(), events_.size(),
                                        1);
    if (trigger_number == -1) {
      if (errno != EINTR) {
        DLOG(WARNING)<< "epoll_wait error";
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
  pthread_t main_tid = loop_tid_;

  loop_pthread_.reset(new StoppableThread(NewCallback(this, &Epoller::Loop)));
  if (!loop_pthread_->Start()) return false;
  while (main_tid == loop_tid_) {
    ::usleep(100);  // FIXME: ugly.
  }

  return true;
}

void Epoller::Stop() {
  if (stop_) return;

  stop_ = true;
  loop_pthread_.reset();
}

uint32 Epoller::ChangeEvent(uint8 event) {
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
  event.events = ChangeEvent(ev->event);
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
  event.events = ChangeEvent(ev->event);
  event.data.ptr = ev;

  int ret = ::epoll_ctl(ep_fd_, EPOLL_CTL_MOD, ev->fd, &event);
  if (ret != 0) {
    PLOG(WARNING)<< "epoll_ctl error";
  }
}

void Epoller::Del(const Event& ev) {
  EvMap::iterator it = ev_map_.find(ev.fd);
  if (it == ev_map_.end()) {
    LOG(WARNING)<< "not registe fd: " << ev.fd;
    return;
  }

  epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = ChangeEvent(ev.event);
  event.data.ptr = ev;

  int ret = ::epoll_ctl(ep_fd_, EPOLL_CTL_DEL, ev.fd, &event);
  if (ret != 0) {
    PLOG(WARNING)<< "EPOLL_CTL_DEL error";
    return;
  }
  ev_map_.erase(it);
}

}
