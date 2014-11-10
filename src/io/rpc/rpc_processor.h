#ifndef RPC_PROCESSOR_H_
#define RPC_PROCESSOR_H_

#include "io/protocol.h"

namespace rpc {
class HandlerMap;

class RpcProcessor : public io::Protocol::Processor {
  public:
    explicit RpcProcessor(HandlerMap* handler_map)
        : handler_map_(handler_map) {
      DCHECK_NOTNULL(handler_map);
    }
    virtual ~RpcProcessor();

  private:
    HandlerMap* handler_map_;

    virtual void dispatch(io::Connection* conn, io::InputBuf* input_buf,
                          const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcProcessor);
};

}

#endif /* RPC_PROCESSOR_H_ */
