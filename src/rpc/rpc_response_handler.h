#ifndef RPC_PROCESSOR_H_
#define RPC_PROCESSOR_H_

#include "rpc_def.h"
#include "rpc_processor.h"

namespace io {
class EventManager;
class OutputObject;
}

namespace rpc {

class RpcClientChannel : public RpcProcessor::Delegate,
    public google::protobuf::RpcChannel {
  public:
    virtual ~RpcClientChannel();

    class Sender {
      public:
        virtual ~Sender() {
        }

        virtual void send(io::OutputObject* object) = 0;
    };
    RpcClientChannel(Sender* sender)
        : sender_(sender) {
      DCHECK_NOTNULL(sender);
    }

  private:
    Sender* sender_;

    // handle response.
    virtual void process(io::Connection* conn, io::InputBuf* input_buf,
                         const TimeStamp& time_stamp);
    // handle request.
    virtual void CallMethod(const MethodDescriptor* method,
                            RpcController* controller, const Message* request,
                            Message* response, google::protobuf::Closure* done);

    void checkTimedout(const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcClientChannel);
};

}

#endif /* RPC_PROCESSOR_H_ */
