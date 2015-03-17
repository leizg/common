#include "acceptor.h"
#include "protocol.h"
#include "tcp_server.h"
#include "connection.h"
#include "event_manager.h"
#include "event_pooler.h"

namespace async {

TcpServer::TcpServer(EventManager* ev_mgr, uint8 worker)
    : MulityTableObjectSaver<int, Connection,
          ThreadSafeObjectSaver<int, Connection, RefCountedObjectMapSaver> >(
        100, false), worker_(worker), ev_mgr_(ev_mgr) {
  protocol_ = nullptr;
  CHECK_NOTNULL(ev_mgr);
  listeners_.reset(new ListenerMap);
}

TcpServer::~TcpServer() {
}

bool TcpServer::Init() {
  if (event_poller_ != nullptr) return false;

  event_poller_.reset(new EventPooler(ev_mgr_, worker_));
  if (!event_poller_->Init()) {
    event_poller_.reset();
    return false;
  }

  return true;
}

bool TcpServer::bindIp(const std::string& ip, uint16 port) {
  DCHECK_NOTNULL(protocol_);

  bool success = false;
  if (ev_mgr_->inValidThread()) {
    bindIpInternal(ip, port, &success, NULL);
    return success;
  }

  SyncEvent event;
  ev_mgr_->runInLoop(
      ::NewCallback(this, &TcpServer::bindIpInternal, ip, port, &success,
                    &event));
  if (!event.TimeWait(3 * TimeStamp::kMicroSecsPerSecond)) {
    return false;
  }
  return success;
}

void TcpServer::unBindIp(const std::string& ip) {
  if (ev_mgr_->inValidThread()) {
    unBindIpInternal(ip, NULL);
    return;
  }

  SyncEvent event;
  ev_mgr_->runInLoop(
      ::NewCallback(this, &TcpServer::unBindIpInternal, ip, &event));
  event.TimeWait(3 * TimeStamp::kMicroSecsPerSecond);
}

void TcpServer::unBindAll() {
  if (ev_mgr_->inValidThread()) {
    unBindAllInternal( NULL);
    return;
  }

  SyncEvent event;
  ev_mgr_->runInLoop(
      ::NewCallback(this, &TcpServer::unBindAllInternal, &event));
  event.TimeWait(3 * TimeStamp::kMicroSecsPerSecond);
}

void TcpServer::bindIpInternal(const std::string ip, uint16 port, bool* success,
                               SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  if (listeners_->Find(ip) != nullptr) {
    *success = true;
    if (ev != nullptr) ev->Signal();
    return;
  }

  Acceptor* a = new Acceptor(ev_mgr_, this);
  a->setProtocol(protocol_);
  if (!a->doBind(ip, port) || !listeners_->Add(ip, a)) {
    delete a;
    if (ev != nullptr) ev->Signal();
    return;
  }

  *success = true;
  if (ev != nullptr) ev->Signal();
}

void TcpServer::unBindAllInternal(SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  listeners_->clear();
  if (ev != nullptr) ev->Signal();
}

void TcpServer::unBindIpInternal(const std::string ip, SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  listeners_->Remove(ip);
  if (ev != nullptr) ev->Signal();
}

void TcpServer::stop() {
  if (ev_mgr_->inValidThread()) {
    stopInternal(NULL);
    return;
  }

  SyncEvent ev;
  ev_mgr_->runInLoop(NewCallback(this, &TcpServer::stopInternal, &ev));
  ev.Wait();
}

void TcpServer::stopInternal(SyncEvent* ev) {
  clear();
  unBindAll();

  if (event_poller_ != nullptr) {
    event_poller_->stop();
    event_poller_.reset();
  }

  if (ev != nullptr) ev->Signal();
}

EventManager* TcpServer::getPoller() {
  CHECK_NOTNULL(event_poller_.get());
  return event_poller_->getPoller();
}

}
