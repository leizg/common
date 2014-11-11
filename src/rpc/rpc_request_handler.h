#ifndef RPC_REQUEST_HANDLER_H_
#define RPC_REQUEST_HANDLER_H_

#include "rpc_processor.h"

namespace rpc {
class HandlerMap;

// TODO: 1. retransmit rpc.
class RpcRequestHandler : public RpcProcessor::Delegate {
  public:
    explicit RpcRequestHandler(HandlerMap* handler_map)
        : handler_map_(handler_map) {
      DCHECK_NOTNULL(handler_map);
    }
    virtual ~RpcRequestHandler();

  private:
    HandlerMap* handler_map_;

    virtual void process(io::Connection* conn, io::InputBuf* input_buf,
                         const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcRequestHandler);
};

}
#endif  // RPC_REQUEST_HANDLER_H_
