#include "protocol.h"
#include "acceptor.h"
#include "connection.h"
#include "event_manager.h"

namespace {

void HandleEvent(int fd, void* arg, uint8 event, const TimeStamp& time_stamp) {
  io::Acceptor* a = static_cast<io::Acceptor*>(arg);
  a->handleAccept();
}

}

namespace io {

Acceptor::~Acceptor() {
  if (listen_fd_ != INVALID_FD) {
    ev_mgr_->Del(*event_);
    ::close(listen_fd_);
  }
}

bool Acceptor::CreateListenFd(const std::string& ip, uint16 port) {
  listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ == -1) {
    PLOG(WARNING)<< "socket error";
    return false;
  }
  setFdNonBlock(listen_fd_);
  setFdCloExec(listen_fd_);

  sockaddr_in addr;
  ::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ::inet_addr(ip.c_str());
  int ret = ::bind(listen_fd_, (sockaddr*) &addr, sizeof(addr));
  if (ret != 0) {
    ::close(listen_fd_);
    listen_fd_ = INVALID_FD;
    PLOG(WARNING)<< "socket bind error: " << ip;
    return false;
  }

  // todo: magic number -> macro def
  ret = ::listen(listen_fd_, 1024);
  if (ret != 0) {
    ::close(listen_fd_);
    listen_fd_ = INVALID_FD;
    PLOG(WARNING)<< "socket listen error";
    return false;
  }

  return true;
}

bool Acceptor::doBind(const std::string& ip, uint16 port) {
  if (listen_fd_ != INVALID_FD) return true;
  if (!CreateListenFd(ip, port)) return false;

  event_.reset(new Event);
  event_->fd = listen_fd_;
  event_->arg = this;
  event_->event = EV_READ;
  event_->cb = HandleEvent;

  if (!ev_mgr_->Add(event_.get())) {
    event_.reset();
    ::close(listen_fd_);
    listen_fd_ = INVALID_FD;
    return false;
  }

  LOG(WARNING)<< "listen ip: " << ip << " port: " << port;
  return true;
}

void Acceptor::handleAccept() {
  int fd;
  while (true) {
#if __linux__
    fd = ::accept4(listen_fd_, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
    fd = ::accept(listen_fd_, NULL, NULL);
#endif
    if (fd == -1) {
      if (errno == EINTR) continue;
      else if (errno != EWOULDBLOCK && errno != EAGAIN) {
        PLOG(WARNING)<< "accept error";
      }
      return;
    }

#if not __linux__
    setFdNonBlock(fd);
    setFdCloExec(fd);
#endif
    EventManager* ev_mgr = serv_->getPoller();
    scoped_ref<Connection> conn(new Connection(fd, ev_mgr));
    conn->setProtocol(protocol_);
    conn->setAttr(protocol_->NewConnectionAttr());
    serv_->Add(conn.get());
    conn->setCloseClosure(
        NewPermanentCallback(serv_, &TcpServer::Remove, conn.get()), false);
    ev_mgr->runInLoop(NewCallback(conn.get(), &Connection::Init));
  }
}
}
