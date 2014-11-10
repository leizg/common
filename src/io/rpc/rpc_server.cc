#include "rpc_server.h"
#include "handler_map.h"
#include "rpc_protocol.h"
#include "server_processor.h"

#include "io/tcp_server.h"
#include "io/event_manager.h"

namespace rpc {

RpcServer::~RpcServer() {
}

void RpcServer::setHandlerMap(HandlerMap* handler_map) {
  handler_map_.reset(handler_map);
}

bool RpcServer::loop(bool in_another_thread) {
  DCHECK_NOTNULL(handler_map_.get());
#if 0
  protocol_.reset(new RpcProtocol);
  ServerProcessor* p = new ServerProcessor(handler_map_.get());
  protocol_->SetProcessor(p);
#endif

  tcp_serv_.reset(new io::TcpServer(ev_mgr_, worker_));
  tcp_serv_->setProtocol(protocol_.get());
  if (!tcp_serv_->bindIp(ip_, port_)) {
    protocol_.reset();
    tcp_serv_.reset();
    return false;
  }

  if (in_another_thread) {
    ev_mgr_->LoopInAnotherThread();
    return true;
  }
  ev_mgr_->Loop();
  return true;
}

}
