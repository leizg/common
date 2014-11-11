#ifndef RPC_CLIENT_H_
#define RPC_CLIENT_H_

#include "rpc_def.h"
#include "base/base.h"
#include <google/protobuf/service.h>

namespace io {
class Protocol;
class TcpClient;
class EventManager;
}

namespace rpc {
class HandlerMap;

// todo: multi-tcp-Connection per Channel.
// RpcClient called by Service::Stub.
class RpcClient : public google::protobuf::RpcChannel {
  public:
    // ev_mgr should have been initialized successfully.
    RpcClient(io::EventManager* ev_mgr, HandlerMap* handler_map);
    virtual ~RpcClient();

    // return false iif timedout or error orrcurred.
    // time_out: unit: seconds
    // ip: it's caller's responsity that make sure ip is good format.
    bool Connect(const std::string& ip, uint16 port, uint32 time_out);

    // not threadsafe.
    // cb will be released by RpcClient.
    void SetReconnectClosure(Closure* cb) {
      reconnect_closure_.reset(cb);
    }

  private:
    scoped_ptr<io::TcpClient> client_;
    scoped_ptr<io::Protocol> protocol_;

    // by google::protobuf::RpcChannel.
    // called by Service::Stub.
    virtual void CallMethod(const MethodDescriptor* method,
                            RpcController* controller, const Message* request,
                            Message* response, google::protobuf::Closure* done);

    void Reconnect();
    // reconnect_closure_ will be called after tcp reconnect successfully.
    scoped_ptr<Closure> reconnect_closure_;

    DISALLOW_COPY_AND_ASSIGN(RpcClient);
};

}

#endif /* RPC_CLIENT_H_ */
