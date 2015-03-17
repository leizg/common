#include "io/io_buf.h"
#include "protocol.h"
#include "connector.h"
#include "connection.h"
#include "event_manager.h"
#include "tcp_client_impl.h"

namespace async {

TcpClient* TcpClient::create(EventManager* ev_mgr, const std::string& ip,
                             uint16 port) {
  return new TcpClientImpl(ev_mgr, ip, port);
}

TcpClientImpl::TcpClientImpl(EventManager* ev_mgr, const std::string& ip,
                             uint16 port)
    : TcpClient(ev_mgr, ip, port) {
}

TcpClientImpl::~TcpClientImpl() {
}

void TcpClientImpl::handleConnectionAbort() {
  ev_mgr_->assertThreadSafe();
  scoped_ref<Connection> conn(conn_.release());

  if (close_closure_.get() != nullptr) {
    close_closure_->Run();
  }
}

void TcpClientImpl::connectInternal(uint32 time_out, bool* success,
                                    SyncEvent* ev) {
  *success = false;
  Connector connector;
  int fd = connector.Connect(ip_, port_, time_out);
  if (fd == INVALID_FD) {
    LOG(WARNING) << "connect error";
    return;
  }

  conn_.reset(new Connection(fd, ev_mgr_));
  conn_->setProtocol(protocol_);
  conn_->setData(protocol_->NewConnectionData());
  conn_->setCloseClosure(
      NewPermanentCallback(this, &TcpClientImpl::handleConnectionAbort));
  if (!conn_->init()) {
    conn_.reset();
    return;
  }

  *success = true;
  if (ev != nullptr) ev->Signal();
}

bool TcpClientImpl::connect(uint32 time_out) {
  DCHECK_NOTNULL(protocol_);
  bool success = false;
  if (ev_mgr_->inValidThread()) {
    connectInternal(time_out, &success);
    return success;
  }

  SyncEvent ev;
  ev_mgr_->runInLoop(
      ::NewCallback(this, &TcpClientImpl::connectInternal, time_out, &success,
                    &ev));
  if (!ev.TimeWait(time_out * TimeStamp::kMicroSecsPerMilliSecond)) {
    return false;
  }
  return success;
}

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

}
