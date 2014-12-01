#include "echo_protocol.h"
#include "io/test/echo_coder.h"

#include "io/input_buf.h"

namespace test {

void EchoProtocol::EchoProcessor::dispatch(io::Connection* conn,
                                           io::InputBuf* input_buf,
                                           const TimeStamp& time_stamp) {
  DLOG(INFO)<< "recv data from client: " << conn->Key() << " at: " << time_stamp.microSecs();
}

EchoProtocol::EchoParser::EchoParser() {
  coder_.reset(new EchoCoder);
}

EchoProtocol::EchoParser::~EchoParser() {
}

bool EchoProtocol::EchoParser::parse(io::Connection* const conn,
                                     io::InputBuf* const input_buf) const {
  bool is_last;
  uint32 data_len;
  if (!coder_->Decode(input_buf, &is_last, &data_len)) {
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

