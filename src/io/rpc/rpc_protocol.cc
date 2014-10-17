#include "io/input_buf.h"

#include "rpc_def.h"
#include "rpc_protocol.h"
#include "rpc_processor.h"

namespace {

}

namespace rpc {

void RpcProtocol::RpcAttr::Init() {
  ::memset(&header_, 0, RPC_HEADER_LENGTH);
  io_stat = io::Protocol::IO_HEADER;
  pending_size = 0;
}

bool RpcProtocol::RpcAttr::Parse(char* buf) {
  // FIXME: endian/bigdian
  ::memcpy(&header_, buf, RPC_HEADER_LENGTH);

  return true;
}

bool RpcProtocol::ParseHeader(io::Connection* conn,
                              io::InputBuf* input_buf) const {
  if (input_buf->size() != RPC_HEADER_LENGTH) {
    return false;
  }

  RpcAttr* attr = dynamic_cast<RpcAttr*>(conn->getAttr());
  CHECK_EQ(attr->pending_size, 0);

  if (!attr->Parse(input_buf->Skip(RPC_HEADER_LENGTH))) {
    DLOG(WARNING)<< "parse header error";
    return false;
  }

  return true;
}

}
