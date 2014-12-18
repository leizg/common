#include "protocol.h"

namespace {

bool RecvPending(net::Connection* conn, net::Connection::Attr* attr) {
  if (attr->pending_size == 0) return true;

  int err_no;
  int32 ret = conn->Recv(attr->pending_size, &err_no);
  if (ret != attr->pending_size) {
    if (ret != -1) {
      attr->pending_size -= ret;
      return false;
    }
    return false;
  }

  attr->pending_size = 0;
  return true;
}

}

namespace net {

bool Protocol::recvData(Connection* conn, Connection::Attr* attr,
                        uint32 data_len) const {
  DCHECK_EQ(attr->pending_size, 0);
  int err_no = 0;

  int32 ret = conn->Recv(data_len, &err_no);
  if (ret == 0) return false;
  else if (ret == -1) {
    if (err_no == EWOULDBLOCK || err_no == EAGAIN) {
      attr->pending_size = data_len;
      return false;
    }
    reporter_->report(conn);
    return false;
  }

  DCHECK_GT(ret, 0);
  attr->pending_size = data_len - ret;
  return attr->pending_size == 0;
}

void Protocol::handleRead(Connection* conn, InputBuf* input_buf,
                          const TimeStamp& time_stamp) const {
  Connection::Attr* attr = conn->getAttr();
  RecvPending(conn, attr);

  DCHECK_EQ(attr->pending_size, 0);
  while (true) {
    switch (attr->io_stat) {  // fall through.
      case IO_START:
        attr->io_stat = IO_HEADER;
        if (!recvData(conn, attr, parser_->headerLength())) {
          return;
        }

      case IO_HEADER:
        attr->io_stat = IO_DATA;
        if (!parser_->parse(conn, input_buf)) {
          reporter_->report(conn);
          return;
        }
        if (!recvData(conn, attr, attr->data_len)) {
          return;
        }

      case IO_DATA:
        if (attr->is_last_pkg) {
          processor_->dispatch(conn, input_buf, time_stamp);
          attr->Init();
          // FIXME: reset input buffer.
          return;
        }
        handlePackage(conn, attr, input_buf);
        attr->io_stat = IO_START;
    }
  }
}

}
