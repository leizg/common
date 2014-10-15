#include "protocol.h"

namespace {

bool RecvPending(io::Connection* conn, io::Connection::Attr* attr) {
  if (attr->pending_size == 0) return true;

  int32 ret = conn->Recv(attr->pending_size);
  if (ret != attr->pending_size) {
    if (ret != 0) {
      if (ret != -1) {
        attr->pending_size -= ret;
      }
    }
    return false;
  }

  attr->pending_size = 0;
  return true;
}

}

namespace io {

int32 Protocol::GetNextSegmentLength(Connection* conn, Connection::Attr* attr,
                                     InputBuf* input_buf) const {
  switch (attr->io_stat) {
    case io::Protocol::IO_HEADER:
      attr->io_stat = io::Protocol::IO_DATA;
      return attr->HeaderLength();

    case io::Protocol::IO_DATA:
      if (!ParseHeader(conn, input_buf)) {
        conn->ShutDown();
        return -1;
      }
      attr->io_stat = io::Protocol::IO_END;
      return attr->pending_size;

    default:
      break;
  }

  return 0;
}

bool Protocol::RecvData(Connection* conn, InputBuf* input_buf) const {
  Connection::Attr* attr = conn->getAttr();

  while (true) {
    if (!RecvPending(conn, attr)) {
      return false;
    }

    CHECK_EQ(attr->pending_size, 0);
    int32 data_len = GetNextSegmentLength(conn, attr, input_buf);
    if (data_len == 0) break;

    int32 ret = conn->Recv(data_len);
    if (ret != data_len) {
      if (ret == -1) {
        if (errno == EINTR) continue;
        return false;
      } else if (ret == 0) {
        return false;
      }

      CHECK_GT(ret, 0);
      attr->pending_size = data_len - ret;
    }
  }

  CHECK_EQ(attr->io_stat, io::Protocol::IO_END);
  return true;
}

void Protocol::handleRead(Connection* conn, InputBuf* input_buf,
                          const TimeStamp& time_stamp) const {
  if (RecvData(conn, input_buf)) {
    processor_->Dispatch(conn, input_buf, time_stamp);

    input_buf->review();
    conn->getAttr()->Init();
  }
}

}
