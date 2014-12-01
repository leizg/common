#include "echo_protocol.h"
#include "io/test/echo_coder.h"

#include "io/input_buf.h"

namespace test {

bool EchoProtocol::EchoParser::parse(io::Connection* const conn,
                                     io::InputBuf* const input_buf) const {
  bool is_last;
  uint32 data_len;
  if (!Decode(input_buf, &is_last, &data_len)) {
    LOG(WARNING)<< "parse header error";
    return false;
  }

  DLOG(INFO)<< "is_last: " << (is_last ? "yes" : "no") << " data_len: " << data_len;

  io::Connection::Attr* attr = conn->getAttr();
  attr->is_last_pkg = is_last;
  attr->data_len = data_len;

  return true;
}

void EchoProtocol::EchoAttr::Init() {
  io_stat = 0;
  pending_size = 0;
  data_len = 0;
  is_last_pkg = false;
}

}
