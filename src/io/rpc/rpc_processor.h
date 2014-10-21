#ifndef RPC_PROCESSOR_H_
#define RPC_PROCESSOR_H_

#include "io/protocol.h"

namespace io {
class TcpClient;
}

namespace rpc {
class HandlerMap;

<<<<<<< HEAD
class ServerProcessor : public io::Protocol::Processor {
 public:
  explicit ServerProcessor(HandlerMap* handler_map)
      : handler_map_(handler_map) {
    CHECK_NOTNULL(handler_map);
  }

  virtual ~ServerProcessor();
=======
class RpcProcessor : public io::Protocol::Processor {
  public:
    explicit RpcProcessor(HandlerMap* handler_map)
        : handler_map_(handler_map) {
      CHECK_NOTNULL(handler_map);
    }

    virtual ~RpcProcessor();
>>>>>>> 87b5d7a5435f3213ae9114a3bb5eaff1e61f9d06

  private:
    HandlerMap* handler_map_;

    virtual void Dispatch(io::Connection* conn, io::InputBuf* input_buf,
                          const TimeStamp& time_stamp);

<<<<<<< HEAD
  DISALLOW_COPY_AND_ASSIGN(ServerProcessor);
};

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

class ClientProcessor : public io::Protocol::Processor {
 public:
  explicit ClientProcessor(io::TcpClient* client)
      : request_id_(0),
        client_(client) {
  }

  virtual ~ClientProcessor();

 private:
  uint64 request_id_;
  io::TcpClient* client_;

  virtual void Dispatch(io::Connection* conn, io::InputBuf* input_buf,
                        const TimeStamp& time_stamp);

  DISALLOW_COPY_AND_ASSIGN(ClientProcessor);
=======
    DISALLOW_COPY_AND_ASSIGN(RpcProcessor);
>>>>>>> 87b5d7a5435f3213ae9114a3bb5eaff1e61f9d06
};

}

#endif /* RPC_PROCESSOR_H_ */
