#include "protocol.h"
#include "connection.h"
#include "event_manager.h"

#include <sys/sendfile.h>

namespace {

void handleEvent(int fd, void* arg, uint8 revent, TimeStamp time_stamp) {
  async::Connection* conn = static_cast<async::Connection*>(arg);

  if (revent & EV_READ) {
    conn->handleRead(time_stamp);
  }
  if (revent & EV_WRITE) {
    conn->handleWrite(time_stamp);
  }
}

void skipData(std::vector<iovec>* iov, int32 len) {
  std::vector<iovec> data;
  for (auto it = iov->begin(); it != iov->end(); ++it) {
    const iovec& io = *it;
    if (len == 0) {
      data.push_back(io);
      continue;
    }

    if (io.iov_len == len) {
      len = 0;
    } else if (io.iov_len > len) {
      iovec left;
      left.iov_base = reinterpret_cast<char*>(io.iov_base) + len;
      left.iov_len -= len;
      data.push_back(left);
    } else {
      len -= io.iov_len;
    }
  }

  iov->swap(data);
}
}

namespace async {

Connection::~Connection() {
  closed_ = true;

  if (event_ != nullptr) {
    ev_mgr_->Del(*event_);
  }
  if (close_closure_ != nullptr) {
    close_closure_->Run();
  }

  closeWrapper(fd_);
}

bool Connection::init() {
  if (event_ != nullptr) return false;

  event_.reset(new Event);
  event_->fd = fd_;
  event_->event = EV_READ;
  event_->arg = this;
  event_->cb = handleEvent;

  if (!ev_mgr_->Add(event_.get())) {
    event_.reset();
    return false;
  }

  return true;
}

void Connection::setData(UserData* data) {
  data_.reset(data);
}

void Connection::handleRead(TimeStamp time_stamp) {
  if (fd_ != INVALID_FD && !closed_) {
    protocol_->handleRead(this, time_stamp);
  }
}

void Connection::handleWrite(TimeStamp time_stamp) {
  if (fd_ != INVALID_FD && !closed_) {
    protocol_->handleWrite(this, time_stamp);
  }
}

bool Connection::read(char* buf, int32* len) {
  if (fd_ == INVALID_FD || closed_) {
    return false;
  }

  int32 ret, left = *len;
  while (left != 0) {
    ret = ::recv(fd_, buf, left, 0);
    if (ret == 0) {
      protocol_->handleClose(this);
      return false;
    } else if (ret < 0) {
      switch (errno) {
        case EWOULDBLOCK:
          break;
        case EINTR:
          continue;
        default:
          protocol_->handleError(this);
          return false;
      }
    }

    DCHECK_GT(ret, 0);
    left -= ret;
    buf += ret;
  }

  *len -= left;
  return true;
}

bool Connection::write(const char* buf, int32* len) {
  std::vector<iovec> iov;
  iov.push_back(iovec(buf, *len));
  return write(iov, len);
}

bool Connection::write(int fd, off_t offset, int32* len) {
  if (fd_ == INVALID_FD || closed_) {
    return false;
  }

  uint32 count = *len;
  off_t pos = offset;
  while (count != 0) {
    int32 ret = ::sendfile(fd_, fd, &pos, count);
    if (ret == -1) {
      switch (errno) {
        case EINTR:
          continue;
        case EWOULDBLOCK:
          break;
        default:
          protocol_->handleError(this);
          return false;
      }
    }

    count -= ret;
  }

  *len = pos - offset;
  return true;
}

bool Connection::write(const std::vector<iovec>& iov, int32* len) {
  if (fd_ == INVALID_FD || closed_) {
    return false;
  }

  std::vector<iovec> data(iov);
  int32 ret, left = *len;
  while (left != 0) {
    ret = ::writev(fd_, &data[0], data.size());
    if (ret == -1) {
      switch (errno) {
        case EINTR:
          continue;
        case EWOULDBLOCK:
          break;
        default:
          protocol_->handleError(this);
          return false;
      }
    }

    left -= ret;
    skipData(&data, ret);
  }

  *len -= left;
  return true;
}

void Connection::updateChannel(uint8 event) {
  event_->event = event;
  ev_mgr_->Mod(event_.get());
}

void Connection::shutDownFromServer() {
  if (closed_) return;

  if (close_closure_ != nullptr) {
    close_closure_->Run();
  }

  ::shutdown(fd_, SHUT_WR);
  closed_ = true;
}

}

