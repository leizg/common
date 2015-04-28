#include "protocol.h"
#include "connector.h"
#include "connection.h"
#include "async_client.h"

#include "io/io_buf.h"
#include "event/event_manager.h"

namespace async {

AsyncClient::~AsyncClient() {
}

void AsyncClient::stopInternal(SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();

  ScopedSyncEvent n(ev);
  if (close_closure_.get() != nullptr) {
    close_closure_->Run();
  }

  conn_.reset();
}

void AsyncClient::stop() {
  if (ev_mgr_->inValidThread()) {
    stopInternal();
    return;
  }

  SyncEvent ev;
  ev_mgr_->runInLoop(::NewCallback(this, &AsyncClient::stopInternal, &ev));
  ev.Wait();
}

void AsyncClient::handleConnectionAbort() {
  ev_mgr_->assertThreadSafe();
  conn_.reset();

  while (true) {
    if (connect(timeout_)) {
      if (reconnect_closure_ != nullptr) {
        reconnect_closure_->Run();
      }
      break;
    }

    // todo: wait policy.
  }
}

bool AsyncClient::connect(uint32 timeout) {
  DCHECK_NOTNULL(protocol_);
  if (conn_ != nullptr) return false;

  timeout_ = timeout;
  bool success = false;
  if (ev_mgr_->inValidThread()) {
    connectInternal(&success);
    return success;
  }

  SyncEvent ev;
  ev_mgr_->runInLoop(
      ::NewCallback(this, &AsyncClient::connectInternal, &success, &ev));
  if (!ev.TimeWait(timeout * TimeStamp::kMicroSecsPerMilliSecond)) {
    return false;
  }
  return success;
}

void AsyncClient::connectInternal(bool* success, SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  *success = false;

  int fd;
  ScopedSyncEvent n(ev);
  if (!doConnect(&fd)) return;
  scoped_ref<Connection> conn(new Connection(fd, ev_mgr_));
  conn->setProtocol(protocol_);
  conn->setData(protocol_->NewConnectionData());
  conn->setCloseClosure(
      ::NewPermanentCallback((AsyncClient*) this,
                             &AsyncClient::handleConnectionAbort));
  if (conn->init()) {
    *success = true;
    conn_.reset(conn.release());
    return;
  }
}

bool TcpAsyncClient::doConnect(int* fd) {
  Connector connector;
  int client_fd = connector.connect(ip_, port_, timeout_);
  if (client_fd == INVALID_FD) {
    return false;
  }

  *fd = client_fd;
  return true;
}

bool LocalAsyncClient::doConnect(int* fd) {
  Connector connector;
  int client_fd = connector.connect(path_, timeout_);
  if (client_fd == INVALID_FD) {
    return false;
  }

  *fd = client_fd;
  return true;
}

void AsyncClient::sendInternal(io::OutputObject* out_obj) {
  ev_mgr_->assertThreadSafe();
  if (conn_ == nullptr) {
    DLOG(INFO)<<"not connected, drop package.";
    delete out_obj;
    return;
  }

  conn_->send(out_obj);
}

void AsyncClient::send(io::OutputObject* out_obj) {
  if (ev_mgr_->inValidThread()) {
    sendInternal(out_obj);
    return;
  }

  ev_mgr_->runInLoop(::NewCallback(this, &AsyncClient::sendInternal, out_obj));
}

}
