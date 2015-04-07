#include "acceptor.h"
#include "protocol.h"
#include "tcp_server.h"
#include "connection.h"
#include "event_manager.h"

namespace {
void handleAcceptEvent(int fd, void* arg, uint8 event, TimeStamp ts) {
  async::TcpAcceptor* a = static_cast<async::TcpAcceptor*>(arg);
  a->handleAccept(ts);
}

class ReassignConnectionClosure : public Closure {
  public:
    ReassignConnectionClosure(async::EventManager* ev_mgr,
                              async::Connection* conn)
        : ev_mgr_(ev_mgr), conn_(conn) {
      conn->Ref();
    }
    virtual ~ReassignConnectionClosure() {
    }

  private:
    async::EventManager* ev_mgr_;
    scoped_ref<async::Connection> conn_;

    void Run() {
      if (!conn_->init()) {
        // handle error;
      }
      delete this;
    }

    DISALLOW_COPY_AND_ASSIGN(ReassignConnectionClosure);
};

}

namespace async {
TcpAcceptor::~TcpAcceptor() {
  if (listen_fd_ != INVALID_FD) {
    ev_mgr_->del(*event_);
    ::close(listen_fd_);
  }
}

bool TcpAcceptor::createListenFd(const std::string& ip, uint16 port) {
  listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ == -1) {
    PLOG(WARNING)<< "socket error";
    return false;
  }

  setFdCloExec(listen_fd_);
  setFdNonBlock(listen_fd_);

  int val = 1;
  int ret = ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &val,
                         sizeof(val));
  if (ret != 0) {
    PLOG(WARNING)<< "setsockopt error";
    closeWrapper(listen_fd_);
    return false;
  }

  sockaddr_in addr;
  ::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ::inet_addr(ip.c_str());
  ret = ::bind(listen_fd_, (sockaddr*) &addr, sizeof(addr));
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

bool TcpAcceptor::doBind(const std::string& ip, uint16 port) {
  if (listen_fd_ != INVALID_FD) return true;
  if (!createListenFd(ip, port)) return false;

  event_.reset(new Event);
  event_->fd = listen_fd_;
  event_->arg = this;
  event_->event = EV_READ;
  event_->cb = handleAcceptEvent;

  if (!ev_mgr_->add(event_.get())) {
    event_.reset();
    ::close(listen_fd_);
    listen_fd_ = INVALID_FD;
    return false;
  }

  LOG(WARNING)<< "listen ip: " << ip << " port: " << port;
  return true;
}

void TcpAcceptor::handleAccept(TimeStamp ts) {
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

#ifndef __linux__
    setFdNonBlock(fd);
    setFdCloExec(fd);
#endif

    EventManager* ev_mgr = serv_->getPoller();
    scoped_ref<Connection> conn(new Connection(fd, ev_mgr));
    conn->setProtocol(protocol_);
    conn->setSaver(serv_);
    conn->setData(protocol_->NewConnectionData());
    serv_->Add(fd, conn.get());

    ev_mgr->runInLoop(new ReassignConnectionClosure(ev_mgr, conn.get()));
  }
}
}
