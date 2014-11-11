#include "rpc_client.h"
#include "io/tcp_client.h"

namespace rpc {

RpcClient::RpcClient(io::EventManager* ev_mgr, HandlerMap* handler_map) {
  RpcProcessor* p = new RpcProcessor(new RpcRequestHandler(handler_map),
                                     new RpcResponseHandler);
  protocol_.reset(new RpcProtocol(p));

  client_.reset(new io::TcpClient(ev_mgr, ip, port));
  client_->SetProtocol(protocol_.get());
}

RpcClient::~RpcClient() {
  client_->SetCloseClosure(NULL);
  client_.reset();
}

bool RpcClient::Connect(uint32 time_out) {
  CHECK(!client_->IsConnected());
  client_->SetCloseClosure(::NewPermanentCallback(this, &RpcClient::Reconnect));

  if (!client_->Connect(time_out)) {
    // TODO:
    return false;
  }

  return true;
}

void RpcClient::CallMethod(const MethodDescriptor* method,
                           RpcController* controller, const Message* request,
                           Message* response, google::protobuf::Closure* done) {
  ClientCallback* cb = dynamic_cast<ClientCallback*>(done);
  DCHECK_NOTNULL(cb);
  cb->SetContext(method, request, response);

  // FIXME:
}

void RpcClient::Reconnect() {
  while (!client_->IsConnected()) {
    if (client_->Connect(3)) {
      LOG(WARNING)<<"reconnect master successfully";
      if (reconnect_closure_ != NULL) {
        reconnect_closure_->Run();
      }
      break;
    }

    DLOG(INFO) << "connect master failed...";
  }
}

}

