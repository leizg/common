#include "io/io_buf.h"
#include "protocol.h"
#include "connector.h"
#include "connection.h"
#include "event_manager.h"
#include "tcp_client_impl.h"

namespace async {

AsyncClient::~AsyncClient() {
}

void TcpAsyncClient::handleConnectionAbort() {
  ev_mgr_->assertThreadSafe();
  {
    scoped_ref<Connection> conn(conn_.release());
    if (close_closure_.get() != nullptr) {
      close_closure_->Run();
    }
  }

  while (!connect(timeout_)) {
    ;  // maybe sleep a while.
  }
}

bool AsyncClient::connect(uint32 timeout) {
  if (conn_ != nullptr) return false;

  timeout_ = timeout;
  DCHECK_NOTNULL(protocol_);
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

void TcpAsyncClient::connectInternal(bool* success, SyncEvent* ev) {
  *success = false;

  ScopedSyncEvent n(ev);
  Connector connector;
  int fd = connector.connect(ip_, port_, timeout_);
  if (fd == INVALID_FD) {
    return;
  }

  scoped_ref<Connection> conn(new Connection(fd, ev_mgr_));
  conn->setProtocol(protocol_);
  conn->setData(protocol_->NewConnectionData());
  conn->setCloseClosure(
      ::NewPermanentCallback((AsyncClient*) this,
                             &AsyncClient::handleConnectionAbort));
  if (conn->init()) {
    *success = true;
    conn_.reset(conn.get());
    return;
  }
}

void LocalAsyncClient::connectInternal(bool* success, SyncEvent* ev = nullptr) {
  *success = false;

  ScopedSyncEvent n(ev);
  Connector connector;
  int fd = connector.connect(path_, timeout_);
  if (fd == INVALID_FD) {
    return;
  }

  scoped_ref<Connection> conn(new Connection(fd, ev_mgr_));
  conn->setProtocol(protocol_);
  conn->setData(protocol_->NewConnectionData());
  conn->setCloseClosure(
      ::NewPermanentCallback((AsyncClient*) this,
                             &AsyncClient::handleConnectionAbort));
  if (conn->init()) {
    *success = true;
    conn_.reset(conn.get());
    return;
  }
}

#if 0
void TcpClientImpl::sendInternal(io::OutputObject* out_obj) {
  if (conn_.get() == nullptr) {
    DLOG(INFO)<<"not connected, drop package.";
    delete out_obj;
    return;
  }

  conn_->send(out_obj);
}

void TcpClientImpl::send(io::OutputObject* out_obj) {
  if (ev_mgr_->inValidThread()) {
    sendInternal(out_obj);
    return;
  }

  ev_mgr_->runInLoop(
      ::NewCallback(this, &TcpClientImpl::sendInternal, out_obj));
}
#endif

}
