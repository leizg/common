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
  tcp_serv_.reset(new io::TcpServer(ev_mgr_, ip_, port_));
  tcp_serv_->setWorker(worker_);
  protocol_.reset(new RpcProtocol(handler_map_));

  if (!tcp_serv_->Start()) {
    LOG(WARNING)<< "tcp server start error";
    return;
  }

  ev_mgr_->LoopInAnotherThread();
}

}
