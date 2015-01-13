#include "io_buf.h"
#include "connector.h"
#include "protocol.h"
#include "connection.h"
#include "tcp_client.h"
#include "event_manager.h"

namespace async {

TcpClient::TcpClient(EventManager* ev_mgr, const std::string& ip, uint16 port)
    : ip_(ip), port_(port), ev_mgr_(ev_mgr), protocol_(NULL) {
}

TcpClient::~TcpClient() {
}

bool TcpClient::Connect(uint32 time_out) {
  CHECK_NOTNULL(protocol_);
  Connector connector;
  int fd = connector.Connect(ip_, port_, time_out);

  ScopedMutex l(&mutex_);
  if (fd != INVALID_FD) {
    connection_.reset(new Connection(fd, ev_mgr_));
    connection_->setProtocol(protocol_);
    connection_->setAttr(protocol_->NewConnectionAttr());
    connection_->setCloseClosure(
        NewPermanentCallback(this, &TcpClient::Remove, connection_.get()));
    connection_->Init();
  }

  return true;
}

void TcpClient::Remove(Connection* conn) {
  {
    ScopedMutex l(&mutex_);
    connection_.reset();
  }

  if (close_closure_.get() != NULL) {
    close_closure_->Run();
  }
}

void TcpClient::Send(OutputObject* io_obj) {
  ScopedMutex l(&mutex_);
  if (connection_ == NULL) {
    DLOG(INFO)<< "not connected, drop package.";
    delete io_obj;
    return;
  }

  if (ev_mgr_->inValidThread()) {
    connection_->Send(io_obj);
    return;
  }

  ev_mgr_->runInLoop(
      ::NewCallback(connection_.get(), &Connection::Send, io_obj));
}

}
