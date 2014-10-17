#ifndef RPC_PROTOCOL_H_
#define RPC_PROTOCOL_H_

#include "rpc_def.h"
#include "io/protocol.h"

namespace rpc {

class RpcProtocol : public io::Protocol {
 public:
  class RpcAttr : public io::Connection::Attr {
   public:
    RpcAttr() {
      Init();
    }
    virtual ~RpcAttr() {
    }

    virtual void Init();
    virtual uint32 HeaderLength() const {
      return RPC_HEADER_LENGTH;
    }

    bool Parse(char* buf);
    const MessageHeader& header() const {
      return header_;
    }

   private:
    MessageHeader header_;

    DISALLOW_COPY_AND_ASSIGN(RpcAttr);
  };

  RpcProtocol() {
  }
  virtual ~RpcProtocol() {
  }

  virtual io::Connection::Attr* NewConnectionAttr() const {
    return new RpcAttr;
  }

 private:

  virtual bool ParseHeader(io::Connection* conn, io::InputBuf* input_buf) const;

  DISALLOW_COPY_AND_ASSIGN(RpcProtocol);
};

inline const MessageHeader& GetRpcHeaderFromConnection(io::Connection* conn) {
  RpcProtocol::RpcAttr* attr =
      dynamic_cast<RpcProtocol::RpcAttr*>(conn->getAttr());
  return attr->header();
}
}

#endif /* RPC_PROTOCOL_H_ */
