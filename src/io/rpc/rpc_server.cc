#include "rpc_server.h"
#include "handler_map.h"
#include "rpc_protocol.h"

#include "io/tcp_server.h"
#include "io/event_manager.h"

namespace rpc {

RpcServer::~RpcServer() {
}

void RpcServer::setHandlerMap(HandlerMap* handler_map) {
  handler_map_.reset(handler_map);
}

void RpcServer::Loop(bool in_another_thread) {
  CHECK_NOTNULL(handler_map_.get());
  protocol_.reset(new RpcProtocol(handler_map_.get()));

  tcp_serv_.reset(new io::TcpServer(ev_mgr_, ip_, port_));
  tcp_serv_->setWorker(worker_);
  tcp_serv_->setProtocol(protocol_.get());
  if (!tcp_serv_->Start()) {
    protocol_.reset();
    tcp_serv_.reset();
    return;
  }

  if (!in_another_thread) {
    ev_mgr_->Loop();
    return;
  }
  ev_mgr_->LoopInAnotherThread();
}

}
