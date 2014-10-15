#include "protocol.h"

namespace io {

bool Protocol::RecvData(Connection* conn, InputBuf* input_buf) const {

}

void Protocol::handleRead(Connection* conn, InputBuf* input_buf,
                          const TimeStamp& time_stamp) const {
  if (RecvData(conn, input_buf)) {
    processor_->Dispatch(conn, input_buf, time_stamp);
    conn->getAttr()->Init();
    input_buf->review();
  }
}

}
