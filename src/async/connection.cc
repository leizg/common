#include "protocol.h"
#include "connection.h"
#include "event_manager.h"
#include "io/io_buf.h"

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

void skipData(std::vector<iovec>* iov, uint32 len) {
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

Connection::Connection(int fd, EventManager* ev_mgr)
    : fd_(fd), closed_(false), ev_mgr_(ev_mgr) {
  DCHECK_NE(fd, INVALID_FD);
  DCHECK_NOTNULL(ev_mgr);
  saver_ = nullptr;
  protocol_ = nullptr;
}

Connection::~Connection() {
  DCHECK(closed_);
  DLOG(INFO)<< "close fd: " << fd_;
  closeWrapper(fd_);
}

bool Connection::init() {
  if (event_ != nullptr) return false;

  event_.reset(new Event);
  event_->fd = fd_;
  event_->event = EV_READ;
  event_->arg = this;
  event_->cb = handleEvent;

  if (!ev_mgr_->add(event_.get())) {
    event_.reset();
    return false;
  }

  out_queue_.reset(new io::OutQueue);
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
    if (out_queue_->empty()) {
      updateChannel(EV_READ);
      protocol_->handleWrite(this, time_stamp);
      return;
    }

    int32 err_no;
    if (out_queue_->send(fd_, &err_no)) {
      updateChannel(EV_READ);
      protocol_->handleWrite(this, time_stamp);
      return;
    }
    if (err_no != EWOULDBLOCK) protocol_->handleClose(this);
  }
}

void Connection::handleClose() {
  if (closed_) return;
  closed_ = true;

  ev_mgr_->del(*event_);
  if (close_closure_ != nullptr) {
    close_closure_->Run();
  }
  if (saver_ != nullptr) {
    saver_->Remove(fd_);
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
      handleClose();
      return false;
    } else if (ret < 0) {
      switch (errno) {
        case EWOULDBLOCK:
          break;
        case EINTR:
          continue;
        default:
          protocol_->handleError(this);
          handleClose();
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

void Connection::send(io::OutputObject* out_obj) {
  if (fd_ == INVALID_FD || closed_) {
    delete out_obj;
    return;
  }
  if (!out_queue_->empty()) {
    out_queue_->push(out_obj);
    return;
  }

  int32 err_no;
  if (out_obj->send(fd_, &err_no)) {
    delete out_obj;
    return;
  }
  if (err_no != EWOULDBLOCK) {
    protocol_->handleClose(this);
    handleClose();
    return;
  }

  out_queue_->push(out_obj);
  updateChannel(EV_READ | EV_WRITE);
}

bool Connection::write(const char* buf, int32* len) {
  std::vector<iovec> iov;
  iovec io;
  io.iov_base = const_cast<char*>(buf);
  io.iov_len = static_cast<uint32>(*len);
  iov.push_back(io);
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
          handleClose();
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
          handleClose();
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
  ev_mgr_->mod(event_.get());
}

void Connection::shutDownFromServer() {
  if (!closed_) {
    ::shutdown(fd_, SHUT_WR);
  }
}

}

