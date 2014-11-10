#ifndef RPC_PROCESSOR_H_
#define RPC_PROCESSOR_H_

#include "io/protocol.h"

namespace rpc {
class HandlerMap;

class RpcProcessor : public io::Protocol::Processor {
  public:
    class ReplyDelegate {
      public:
        virtual ~ReplyDelegate() {
        }

        virtual void handleResponse(io::Connection* conn,
                                    io::InputBuf* input_buf,
                                    const TimeStamp& time_stamp) = 0;
    };

    explicit RpcProcessor(HandlerMap* handler_map, ReplyDelegate* responser)
        : handler_map_(handler_map), reply_delegate_(responser) {
      DCHECK_NOTNULL(handler_map);
      DCHECK_NOTNULL(responser);
    }
    virtual ~RpcProcessor() {
    }

  private:
    HandlerMap* handler_map_;
    scoped_ptr<ReplyDelegate> reply_delegate_;

    virtual void dispatch(io::Connection* conn, io::InputBuf* input_buf,
                          const TimeStamp& time_stamp);

    void handleRequest(io::Connection* conn, io::InputBuf* input_buf,
                       const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcProcessor);
};

}

#endif /* RPC_PROCESSOR_H_ */
