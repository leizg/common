#include "protocol.h"
#include "connection.h"
#include "event/event_manager.h"

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
  for (auto it : *iov) {
    const iovec& io = it;
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

  return true;
}

void Connection::handleRead(TimeStamp time_stamp) {
  if (fd_ != INVALID_FD && !closed_) {
    protocol_->handleRead(this, time_stamp);
  }
}

void Connection::handleWrite(TimeStamp time_stamp) {
  if (fd_ != INVALID_FD && !closed_) {
    int err_no;
    if (!data_->out_queue->empty()) {
      if (!data_->out_queue->send(fd_, &err_no)) {
        if (err_no != EWOULDBLOCK) {
          handleClose();
        }
        return;
      }
    }

    if (protocol_->handleWrite(this, time_stamp, &err_no)) {
      updateChannel(EV_READ);
      return;
    }

    if (err_no == EWOULDBLOCK && !(EV_WRITE & event_->event)) {
      updateChannel(EV_READ | EV_WRITE);
    }
  }
}

void Connection::handleClose() {
  if (closed_) return;
  closed_ = true;

  ev_mgr_->del(*event_);
  if (close_closure_ != nullptr) {
    close_closure_->Run();
  }
}

bool Connection::read(char* buf, int32* len) {
  if (fd_ == INVALID_FD || closed_) return false;

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

Connection::UserData::UserData()
    : conn(nullptr) {
  out_queue.reset(new io::OutQueue);
}

Connection::UserData::~UserData() {
}

void Connection::send(io::OutputObject* obj) {
  if (closed_) {
    delete obj;
    return;
  }
  if (event_->event & EV_WRITE) {
    data_->out_queue->push(obj);
    updateChannel(EV_READ | EV_WRITE);
    return;
  }

  int err_no;
  if (obj->send(fd_, &err_no)) {
    delete obj;
    return;
  }

  if (err_no == EWOULDBLOCK) {
    data_->out_queue->push(obj);
    updateChannel(EV_READ | EV_WRITE);
    return;
  }

  shutDown();
}

bool Connection::write(const char* buf, int32* len, int* err_no) {
  iovec io;
  io.iov_base = const_cast<char*>(buf);
  io.iov_len = static_cast<uint32>(*len);
  std::vector<iovec> vec;
  vec.push_back(io);
  return write(vec, len, err_no);
}

bool Connection::write(int fd, off_t offset, int32* len, int* err_no) {
  if (fd_ == INVALID_FD || closed_) {
    if (err_no != nullptr) *err_no = EBADFD;
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
          if (err_no != nullptr) *err_no = errno;
          return false;
      }
    }

    count -= ret;
  }

  *len = pos - offset;
  return true;
}

bool Connection::write(const std::vector<iovec>& iov, int32* len, int* err_no) {
  if (fd_ == INVALID_FD || closed_) {
    if (err_no != nullptr) *err_no = EBADFD;
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
          if (err_no != nullptr) *err_no = errno;
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
  if (!closed_) {
    event_->event = event;
    ev_mgr_->mod(event_.get());
  }
}

void Connection::shutDown() {
  if (closed_) return;
  if (ev_mgr_->inValidThread()) {
    shutDownInternal();
    return;
  }

  SyncEvent ev;
  ev_mgr_->runInLoop(
      ::NewPermanentCallback(this, &Connection::shutDownInternal, &ev));
  ev.Wait();
}

void Connection::shutDownInternal(SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  handleClose();
  if (ev != nullptr) ev->Signal();
}

}

