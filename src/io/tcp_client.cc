#include "connector.h"
#include "tcp_client.h"
#include "event_manager.h"

namespace io {

TcpClient::TcpClient(EventManager* ev_mgr, const std::string& ip, uint16 port)
    : ip_(ip), port_(port), ev_mgr_(ev_mgr), protocol_(NULL) {
  connector_.reset(new io::Connector);
}

TcpClient::~TcpClient() {
}

bool TcpClient::Connect(uint32 time_out) {
  ScopedMutex l(&mutex_);

  CHECK_NOTNULL(protocol_);
  int fd = connector_->Connect(ip_, port_, time_out);
  if (fd == INVALID_FD) return false;

  connection_.reset(new Connection(fd, ev_mgr_));
  connection_->setProtocol(protocol_);
  connection_->setAttr(protocol_->NewConnectionAttr());
  connection_->setCloseClosure(
      NewPermanentCallback(this, &TcpClient::Remove, connection_.get()));
  connection_->Init();

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
    delete io_obj;
    return;
  }

  if (ev_mgr_->inLoopThread()) {
    connection_->Send(io_obj);
    return;
  }

  ev_mgr_->runInLoop(NewCallback(connection_.get(), &Connection::Send, io_obj));
}

}
