#include "rpc_server.h"
#include "handler_map.h"
#include "rpc_protocol.h"

#include "io/tcp_server.h"
#include "io/event_manager.h"

namespace rpc {

RpcServer::~RpcServer() {
}

void RpcServer::setHandlerMap(HandlerMap* handler_map) {
  DCHECK_NOTNULL(handler_map_.get());
  handler_map_.reset(handler_map);
}

bool RpcServer::start() {
  DCHECK_NOTNULL(handler_map_.get());
  DCHECK_NOTNULL(protocol_.get());
  protocol_.reset(
      new RpcProtocol(
          new RpcProcessor(new RpcRequestHandler(handler_map_.get()),
          NULL)));

  tcp_serv_.reset(new io::TcpServer(ev_mgr_, worker_));
  tcp_serv_->setProtocol(protocol_.get());
  if (!tcp_serv_->bindIp(ip_, port_)) {
    protocol_.reset();
    tcp_serv_.reset();
    return false;
  }

  return true;
}
}
