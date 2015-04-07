#include "acceptor.h"
#include "protocol.h"
#include "connection.h"
#include "event_pooler.h"
#include "async_server.h"
#include "event_manager.h"

namespace async {

AsyncServer::~AsyncServer() {
  stop();
}

void AsyncServer::stopInternal(SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();

  if (listerner_ != nullptr) {
    listerner_->stop();
    listerner_ = nullptr;
  }

  {
    ScopedMutex l(&mutex_);
    for (auto it : map_) {
      Connection* conn = it.second;
      conn->shutDownFromServer();
      conn->UnRef();
    }
    map_.clear();
  }

  if (event_poller_ != nullptr) {
    event_poller_->stop();
    event_poller_.reset();
  }

  if (ev != nullptr) ev->Signal();
}

void AsyncServer::stop() {
  if (ev_mgr_->inValidThread()) {
    stopInternal(NULL);
    return;
  }

  SyncEvent ev;
  ev_mgr_->runInLoop(NewCallback(this, &AsyncServer::stopInternal, &ev));
  ev.Wait();
}

bool AsyncServer::init() {
  if (event_poller_ != nullptr) return false;

  event_poller_.reset(new EventPooler(ev_mgr_, worker_));
  if (!event_poller_->Init()) {
    event_poller_.reset();
    return false;
  }

  return true;
}

bool AsyncServer::bindIp(const std::string& ip, uint16 port) {
  DCHECK_NOTNULL(protocol_);

  bool success = false;
  if (ev_mgr_->inValidThread()) {
    bindIpInternal(ip, port, &success, NULL);
    return success;
  }

  SyncEvent event;
  ev_mgr_->runInLoop(
      ::NewCallback(this, &AsyncServer::bindIpInternal, ip, port, &success,
                    &event));
  if (!event.TimeWait(3 * TimeStamp::kMicroSecsPerSecond)) {
    return false;
  }
  return success;
}

void AsyncServer::unBindIp(const std::string& ip) {
  if (ev_mgr_->inValidThread()) {
    unBindIpInternal(ip, NULL);
    return;
  }

  SyncEvent event;
  ev_mgr_->runInLoop(
      ::NewCallback(this, &AsyncServer::unBindIpInternal, ip, &event));
  event.TimeWait(3 * TimeStamp::kMicroSecsPerSecond);
}

void AsyncServer::unBindAll() {
  if (ev_mgr_->inValidThread()) {
    unBindAllInternal( NULL);
    return;
  }

  SyncEvent event;
  ev_mgr_->runInLoop(
      ::NewCallback(this, &AsyncServer::unBindAllInternal, &event));
  event.TimeWait(3 * TimeStamp::kMicroSecsPerSecond);
}

void AsyncServer::bindIpInternal(const std::string ip, uint16 port,
                                 bool* success, SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  if (listeners_->Find(ip) != nullptr) {
    *success = true;
    if (ev != nullptr) ev->Signal();
    return;
  }

  TcpAcceptor* a = new TcpAcceptor(ev_mgr_, this);
  a->setProtocol(protocol_);
  if (!a->doBind(ip, port) || !listeners_->Add(ip, a)) {
    delete a;
    if (ev != nullptr) ev->Signal();
    return;
  }

  *success = true;
  if (ev != nullptr) ev->Signal();
}

void AsyncServer::unBindAllInternal(SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  listeners_->clear();
  if (ev != nullptr) ev->Signal();
}

void AsyncServer::unBindIpInternal(const std::string ip, SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  listeners_->Remove(ip);
  if (ev != nullptr) ev->Signal();
}

EventManager* AsyncServer::getPoller() {
  CHECK_NOTNULL(event_poller_.get());
  return event_poller_->getPoller();
}

}
