#include "io/io_buf.h"
#include "io/input_stream.h"

#include "protocol.h"
#include "connection.h"
#include "event_manager.h"

DEFINE_int32(input_buf_len, 128, "the size of inputbuf");

namespace {

void handleEvent(int fd, void* arg, uint8 revent, const TimeStamp& time_stamp) {
  aync::Connection* conn = static_cast<aync::Connection*>(arg);

  if (revent & EV_READ) {
    conn->handleRead(time_stamp);
  }
  if (revent & EV_WRITE) {
    conn->handleWrite(time_stamp);
  }
}

}

namespace aync {

// default implemetion.
void Connection::Attr::Init() {
  io_stat = 0;
  pending_size = 0;
  data_len = 0;
  is_last_pkg = false;
}

Connection::Connection(int fd, EventManager* ev_mgr)
    : fd_(fd), closed_(false), ev_mgr_(ev_mgr), saver_(NULL), protocol_(NULL) {
  DCHECK_NE(fd, INVALID_FD);
  DCHECK_NOTNULL(ev_mgr);
}

Connection::~Connection() {
  if (event_.get() != NULL) {
    ev_mgr_->Del(*event_);
  }
  closeWrapper(fd_);
}

void Connection::Init() {
  if (event_.get() != NULL) return;

  event_.reset(new Event);
  event_->fd = fd_;
  event_->event = EV_READ;
  event_->arg = this;
  event_->cb = handleEvent;

  if (!ev_mgr_->Add(event_.get())) {
    event_.reset();
    if (close_closure_.get() != NULL) {
      close_closure_->Run();
    }
    return;
  }

  out_queue_.reset(new io::OutQueue);
  input_stream_.reset(new io::InputStream(FLAGS_input_buf_len));
}

void Connection::setAttr(Attr* attr) {
  attr_.reset(attr);
}

void Connection::Send(io::OutputObject* out_obj) {
  if (fd_ == INVALID_FD || closed_) {
    delete out_obj;
    return;
  }

  int32 err_no;
  if (out_queue_->empty()) {
    if (out_obj->send(fd_, &err_no)) {
      delete out_obj;
      return;
    } else if (err_no != EWOULDBLOCK) {
      delete out_obj;
      ShutDown();
      return;
    }
  }

  out_queue_->push(out_obj);
  if (!(event_->event & EV_WRITE)) {
    event_->event |= EV_WRITE;
    ev_mgr_->Mod(event_.get());
  }
}

int32 Connection::Recv(uint32 len, int* err_no) {
  if (fd_ == INVALID_FD || closed_) return 0;

  int32 ret, left = len;
  while (left != 0) {
    ret = input_stream_->ReadFd(fd_, left, err_no);
    if (ret == 0) {
      ShutDown();
      return 0;
    } else if (ret == -1) {
      if (*err_no == EWOULDBLOCK) {
        break;
      }
      ShutDown();
      return -1;
    }

    DCHECK_GT(ret, 0);
    left -= ret;
  }

  return len - left;
}

void Connection::handleRead(const TimeStamp& time_stamp) {
  if (fd_ != INVALID_FD && !closed_) {
    protocol_->handleRead(this, input_stream_.get(), time_stamp);
  }
}

void Connection::handleWrite(const TimeStamp& time_stamp) {
  if (fd_ == INVALID_FD || closed_) return;

  if (out_queue_->empty()) {
    event_->event &= ~EV_WRITE;
    ev_mgr_->Mod(event_.get());
    return;
  }

  int err_no;
  if (out_queue_->send(fd_, &err_no)) {
    event_->event &= ~EV_WRITE;
    ev_mgr_->Mod(event_.get());
    return;
  }

  if (err_no != EWOULDBLOCK) {
    ShutDown();
  }
}

void Connection::ShutDown() {
  if (closed_) return;
  ::shutdown(fd_, SHUT_WR);

  if (close_closure_.get() != NULL) {
    close_closure_->Run();
  }
  closed_ = true;
}

}

