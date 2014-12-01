#include "io/tcp_server.h"
#include "io/event_manager.h"

#include "echo_server.h"
#include "echo_dispatcher.h"
#include "io/test/echo_protocol.h"

namespace test {

EchoServer::EchoServer(uint32 worker)
    : worker_(worker) {
  DCHECK_GE(worker, 0);
}

EchoServer::~EchoServer() {
}

bool EchoServer::Init(const std::string& ip, uint16 port) {
  protocol_.reset(new EchoProtocol(new EchoDispatcher));
  ev_mgr_.reset(io::CreateEventManager());
  DCHECK_NOTNULL(ev_mgr_.get());
  if (!ev_mgr_->Init()) {
    ev_mgr_.reset();
    return false;
  }

  server_.reset(new io::TcpServer(ev_mgr_.get(), worker_));
  server_->setProtocol(protocol_.get());
  if (!server_->bindIp(ip, port)) {
    ev_mgr_.reset();
    return false;
  }

  return true;
}

void EchoServer::loop(bool in_another_thread) {
  DCHECK(ev_mgr_.get());
  if (!in_another_thread) {
    ev_mgr_->Loop();
    return;
  }

  ev_mgr_->LoopInAnotherThread();
}

}

