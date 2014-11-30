#include "acceptor.h"
#include "protocol.h"
#include "tcp_server.h"
#include "connection.h"
#include "event_manager.h"
#include "event_pooler.h"

namespace io {

TcpServer::TcpServer(EventManager* ev_mgr, uint8 worker)
    : worker_(worker), ev_mgr_(ev_mgr), protocol_(
    NULL), saver_(new RefCountedObjectSavePolicy<Connection>) {
  CHECK_NOTNULL(ev_mgr);
}

TcpServer::~TcpServer() {
  unBindAll();
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
  ScopedMutex l(&mutex_);
  if (listeners_.count(ip) != 0) return true;

  Acceptor* a = new Acceptor(ev_mgr_, this);
  a->setProtocol(protocol_);
  if (!a->doBind(ip, port)) {
    delete a;
    return false;
  }
  listeners_[ip] = a;

  return true;
}

void TcpServer::unBindIp(const std::string& ip) {
  ScopedMutex l(&mutex_);
  MapEarseAndDelete(&listeners_, ip);
}

void TcpServer::unBindAll() {
  ScopedMutex l(&mutex_);
  STLMapClear(&listeners_);
}

EventManager* TcpServer::getPoller() {
  CHECK_NOTNULL(event_poller_.get());
  return event_poller_->getPoller();
}

}
