#include "event_pipe.h"

namespace io {

bool EventPipe::initPipe() {
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

#ifndef __linux__
  for (uint32 i = 0; i < 2; ++i) {
    setFdCloExec(event_fd_[i]);
    setFdNonBlock(event_fd_[i]);
  }
#endif
  return true;
}

void EventPipe::destory() {
  if (event_fd_[0] != INVALID_FD) {
    ::close(event_fd_[0]);
    event_fd_[0] = INVALID_FD;
  }

  if (event_fd_[1] != INVALID_FD) {
    ::close(event_fd_[1]);
    event_fd_[1] = INVALID_FD;
  }
}

void EventPipe::triggerPipe() {
  uint8 c;
  ::send(event_fd_[1], &c, sizeof(c), 0);
}

void EventPipe::handlePipeRead() {
  DCHECK_NOTNULL(deletate_.get());
  uint64 dummy;
  while (true) {  // read all of data.
    int ret = ::recv(event_fd_[0], &dummy, sizeof(dummy), 0);
    if (ret == -1) {
      switch (errno) {
        case EINTR:
          continue;
        case EWOULDBLOCK:
          break;
        default:
          PLOG(WARNING)<< "read pipe error";
          break;
        }
      }
    }

  deletate_->handlevent();
}
}
