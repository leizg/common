#ifndef RPC_PROTOCOL_H_
#define RPC_PROTOCOL_H_

#include "rpc_def.h"
#include "io/protocol.h"

namespace rpc {

namespace detail {
class RpcAttr : public io::Connection::Attr {
  public:
    RpcAttr() {
      Init();
    }
    virtual ~RpcAttr() {
    }

    virtual void Init() {
      io::Connection::Attr::Init();
      ::memset(&header_, 0, sizeof(header_));
    }

    MessageHeader* header() const {
      return &header_;
    }

  private:
    MessageHeader header_;

    DISALLOW_COPY_AND_ASSIGN(RpcAttr);
};

class RpcParser : io::Protocol::Parser {
  public:
    RpcParser() {
    }
    virtual ~RpcParser() {
    }

  private:
    virtual uint32 headerLength() const {
      return RPC_HEADER_LENGTH;
    }

    virtual bool parse(io::Connection* const conn,
                       io::InputBuf* const input_buf) const;

    DISALLOW_COPY_AND_ASSIGN(RpcParser);
};
}

class RpcProtocol : public io::Protocol {
  public:
    explicit RpcProtocol(Processor* p)
        : io::Protocol(p, new detail::RpcParser) {
    }
    virtual ~RpcProtocol() {
    }

  private:
    virtual io::Connection::Attr* NewConnectionAttr() const {
      return new detail::RpcAttr;
    }

    DISALLOW_COPY_AND_ASSIGN(RpcProtocol);
};

inline MessageHeader* GetRpcHeaderFromConnection(io::Connection* conn) {
  detail::RpcAttr* attr = dynamic_cast<detail::RpcAttr*>(conn->getAttr());
  return attr->header();
}
}

#endif /* RPC_PROTOCOL_H_ */
