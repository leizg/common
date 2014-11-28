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

    class ReplyClosure : public ::google::protobuf::Closure {
      public:
        ReplyClosure(io::Connection* conn, const MessageHeader& header,
                     Message* reply);
        virtual ~ReplyClosure();

      private:
        const MessageHeader hdr_;
        scoped_ptr<Message> reply_;

        scoped_ref<io::Connection> conn_;

        virtual void Run();

        DISALLOW_COPY_AND_ASSIGN(ReplyClosure);
    };

    DISALLOW_COPY_AND_ASSIGN(RpcRequestHandler);
};
}
#endif  // RPC_REQUEST_HANDLER_H_
