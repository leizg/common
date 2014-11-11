#ifndef RPC_PROCESSOR_H_
#define RPC_PROCESSOR_H_

#include "rpc_processor.h"

namespace io {
class TcpClient;
}

namespace rpc {

class RpcContext : public LinkNode {
  public:
    RpcContext(const TimeStamp& time_stamp, uint64 re_id);
    virtual ~RpcContext();

    uint64 id() const {
      return req_id_;
    }

    void SetContext(uint32 method_id, const Message* request, Message* response,
                    google::protobuf::Closure* done);

  private:
    const uint64 req_id_;

    uint32 method_id_;
    const Message* request_;
    Message* response_;
    google::protobuf::Closure* done_;

    DISALLOW_COPY_AND_ASSIGN(RpcContext);
};

class RpcResponseHandler : public RpcProcessor::Delegate {
  public:
    RpcResponseHandler() {
    }
    virtual ~RpcResponseHandler();

  private:
    virtual void process(io::Connection* conn, io::InputBuf* input_buf,
                                const TimeStamp& time_stamp);

    void checkTimedout(const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcResponseHandler);
};

}

#endif /* RPC_PROCESSOR_H_ */
