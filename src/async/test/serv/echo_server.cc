#include "async/async_server.h"
#include "async/event/event_manager.h"

#include "echo_server.h"
#include "echo_dispatcher.h"
#include "async/test/echo_protocol.h"

namespace test {

EchoServer::EchoServer(uint32 worker)
    : worker_(worker) {
  DCHECK_GE(worker, 0);
}

EchoServer::~EchoServer() {
  DLOG(INFO)<< "stop tcp server";
  if (server_ != nullptr) {
    server_->stop();
    server_.reset();
  }

  DLOG(INFO)<< "stop main thread";
  if (ev_mgr_ != nullptr) {
    SyncEvent ev;
    ev_mgr_->stop(&ev);
    ev.Wait();
  }
}

bool EchoServer::init(const std::string& ip, uint16 port) {
  protocol_.reset(new EchoProtocol(new EchoDispatcher));
  ev_mgr_.reset(async::CreateEventManager());
  DCHECK_NOTNULL(ev_mgr_.get());
  if (!ev_mgr_->init()) {
    ev_mgr_.reset();
    return false;
  }

  int server_fd;
  if (!async::createTcpServer(ip, port, &server_fd)) {
    ev_mgr_.reset();
    return false;
  }
  server_.reset(new async::AsyncServer(ev_mgr_.get(), server_fd, worker_));
  server_->setProtocol(protocol_.get());
  if (!server_->init()) {
    ev_mgr_.reset();
    server_.reset();
    return false;
  }

  return true;
}

void EchoServer::loop(bool in_another_thread) {
  DCHECK_NOTNULL(ev_mgr_.get());
  if (!in_another_thread) {
    ev_mgr_->loop();
    return;
  }

  ev_mgr_->loopInAnotherThread();
}

}

