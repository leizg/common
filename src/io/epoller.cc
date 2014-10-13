#include "epoller.h"
#include <sys/epoll.h>

namespace io {

Epoller::~Epoller() {
  if (ep_fd_ != INVALID_FD) {
    ::close(ep_fd_);
  }
}

bool Epoller::Init() {
  ep_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
  if (ep_fd_ == INVALID_FD) {
    PLOG(WARNING)<< "epoll_create error";
    return false;
  }

  SetFdNonBlock(ep_fd_);

  return InitPipe();
}

void Epoller::Loop() {
  stop_ = false;

  while (!stop_) {
    // TODO:
  }
}

bool Epoller::LoopInAnotherThread() {
  pthread_t main_tid = loop_tid_;

  loop_pthread_.reset(new StoppableThread(NewCallback(this, &Epoller::Loop)));
  if (!loop_pthread_->Start()) return false;
  while (main_tid == loop_tid_) {
    ::usleep(100); // FIXME: ugly.
  }

  return true;
}

void Epoller::Stop() {
  if (stop_) return;

  stop_ = true;
  loop_pthread_.reset();
}

bool Epoller::Add(Event* ev) {
  CHECK_NOTNULL(ev);
}

void Epoller::Mod(Event* ev) {
  CHECK_NOTNULL(ev);
}

void Epoller::Del(const Event& ev) {
}

}
