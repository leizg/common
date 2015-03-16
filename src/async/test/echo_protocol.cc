#include "echo_protocol.h"
#include "async/connection.h"
#include "async/test/echo_coder.h"

#include "io/input_stream.h"

namespace test {

bool EchoProtocol::EchoParser::parseHeader(async::Connection* conn) const {
  ProReactorProtocol::UserData* ud =
      static_cast<ProReactorProtocol::UserData*>(conn->getData());
  char* data = ud->peekHeader();

  bool is_last;
  uint32 data_len;
  if (!Decode(data, &is_last, &data_len)) {
    LOG(WARNING)<< "parse header error";
    return false;
  }

  DLOG(INFO)<< "is_last: " << (is_last ? "yes" : "no") << " data_len: " << data_len;

  ud->is_last = is_last;
  ud->pending_size = data_len;

  return true;
}

}

