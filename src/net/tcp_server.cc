#include "acceptor.h"
#include "protocol.h"
#include "tcp_server.h"
#include "connection.h"
#include "event_manager.h"
#include "event_pooler.h"

namespace net {

TcpServer::TcpServer(EventManager* ev_mgr, uint8 worker)
    : MulityTableObjectSaver<int, Connection,
          ThreadSafeObjectSaver<int, Connection, RefCountedObjectMapSaver> >(
        100, false), worker_(worker), ev_mgr_(ev_mgr), protocol_(
    NULL) {
  CHECK_NOTNULL(ev_mgr);

  listeners_.reset(new ListenerMap);
}

TcpServer::~TcpServer() {
  unBindAll();

  // todo: remote all connections.
}

bool TcpServer::Init() {
  if (event_poller_.get() != NULL) return false;

  event_poller_.reset(new EventPooler(ev_mgr_, worker_));
  if (!event_poller_->Init()) {
    event_poller_.reset();
    return false;
  }

  return true;
}

bool TcpServer::bindIp(const std::string& ip, uint16 port) {
  CHECK_NOTNULL(protocol_);
  if (listeners_->Find(ip) != nullptr) return true;

  Acceptor* a = new Acceptor(ev_mgr_, this);
  a->setProtocol(protocol_);
  if (!a->doBind(ip, port) || !listeners_->Add(ip, a)) {
    delete a;
    return false;
  }

  return true;
}

void TcpServer::unBindIp(const std::string& ip) {
  listeners_->Remove(ip);
}

void TcpServer::unBindAll() {
  listeners_->clear();
}

EventManager* TcpServer::getPoller() {
  CHECK_NOTNULL(event_poller_.get());
  return event_poller_->getPoller();
}

}
