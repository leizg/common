#ifndef RPC_CLIENT_H_
#define RPC_CLIENT_H_

#include <google/protobuf/service.h>

#include "rpc_def.h"
#include "base/base.h"

namespace io {
class TcpClient;
class EventManager;
}

namespace rpc {

class RpcClient : public google::protobuf::RpcChannel {
  public:
    RpcClient(io::EventManager* ev_mgr, const std::string& ip, uint16 port);
    virtual ~RpcClient();

    bool Connect(uint32 time_out);

    void SetReconnectClosure(Closure* cb);

  private:
    scoped_ptr<io::TcpClient> client_;

    // by google::protobuf::RpcChannel.
    virtual void CallMethod(const MethodDescriptor* method,
                            RpcController* controller, const Message* request,
                            Message* response, google::protobuf::Closure* done);
    void Reconnect();

    DISALLOW_COPY_AND_ASSIGN(RpcClient);
};

}

#endif /* RPC_CLIENT_H_ */
