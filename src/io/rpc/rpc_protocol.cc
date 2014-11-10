#include "io/input_buf.h"

#include "rpc_def.h"
#include "rpc_protocol.h"
#include "rpc_processor.h"

namespace {

}

namespace rpc {

bool detail::RpcParser::parse(io::Connection* const conn,
                              io::InputBuf* const input_buf) const {
  // FIXME: check package length.
#if 0
  if (input_buf->size() < RPC_HEADER_LENGTH) {
    LOG(WARNING)<<"header is too small.";
    return false;
  }
#endif

  RpcAttr* attr = dynamic_cast<RpcAttr*>(conn->getAttr());
  CHECK_EQ(attr->pending_size, 0);

  char* data = input_buf->Skip(RPC_HEADER_LENGTH);
  MessageHeader* hdr = attr->header();
  // FIXME: big edian.
  ::memcpy(hdr, data, RPC_HEADER_LENGTH);

  attr->data_len = hdr->length;
  attr->is_last_pkg = IS_LAST_PACKAGE(*hdr);
  return true;
}

RpcProtocol::RpcProtocol(HandlerMap* handler_map)
    : io::Protocol(new RpcProcessor(handler_map), new detail::RpcParser) {
}
}
