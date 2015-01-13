#include "event_pipe.h"

namespace async {

bool EventPipe::initPipe() {
  int ret;
#if __linux__
  ret = ::pipe2(event_fd_, O_NONBLOCK | O_CLOEXEC);
#else
  ret = ::pipe(event_fd_);
#endif
  if (ret == -1) {
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
  for (uint32 i = 0; i < 2; ++i) {
    closeWrapper(event_fd_[i]);
  }
}

void EventPipe::triggerPipe() {
  uint8 c;
  int ret = write(event_fd_[1], &c, sizeof(c));
  CHECK_EQ(ret, sizeof(c))<< event_fd_[1] << " " << strerror(errno);
}

void EventPipe::handlePipeRead() {
  uint8 dummy;
  while (true) {  // read all of data.
    int ret = ::read(event_fd_[0], &dummy, sizeof(dummy));
    if (ret == -1) {
      switch (errno) {
        case EINTR:
        continue;
        case EWOULDBLOCK:
        return;
      }
      PLOG(WARNING)<< "read pipe error" << event_fd_[0];
      return;
    }
    break;
  }
  deletate_->handleEvent();
}
}
