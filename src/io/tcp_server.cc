#include "acceptor.h"
#include "protocol.h"
#include "tcp_server.h"
#include "connection.h"
#include "event_manager.h"

namespace io {

TcpServer::TcpServer(EventManager* ev_mgr, const std::string& ip, uint16 port)
    : ip_(ip),
      port_(port),
      worker_(0),
      ev_mgr_(ev_mgr),
      protocol_(NULL) {
  CHECK_NOTNULL(ev_mgr);
  CHECK(!ip.empty());
}

TcpServer::~TcpServer() {
}

bool TcpServer::Start() {
  CHECK_NOTNULL(protocol_);
  if (listener_.get() != NULL) return true;

  event_poller_.reset(new EventPooler(ev_mgr_, worker_));
  if (!event_poller_->Init()) {
    event_poller_.reset();
    return false;
  }

  Acceptor* a = new Acceptor(ev_mgr_, this);
  a->setProtocol(protocol_);
  if (!a->Init(ip_, port_)) {
    delete a;
    event_poller_.reset();
    return false;
  }
  listener_.reset(a);

  return true;
}

void TcpServer::Stop() {
  listener_.reset();
  event_poller_.reset();
}

EventManager* TcpServer::getPoller() {
  CHECK_NOTNULL(event_poller_.get());
  return event_poller_->getPoller();
}

void TcpServer::Add(Connection* conn) {
  conn->Ref();

  ScopedMutex l(&mutex_);
  CHECK_EQ(conn_map_.count(conn->FileHandle()), 0);
  conn_map_[conn->FileHandle()] = conn;
}

void TcpServer::Remove(Connection* conn) {
  {
    ScopedMutex l(&mutex_);
    CHECK_EQ(conn_map_.count(conn->FileHandle()), 1);
    conn_map_.erase(conn->FileHandle());
  }

  conn->UnRef();
}

}
