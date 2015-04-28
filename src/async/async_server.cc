#include <sys/un.h>

#include "protocol.h"
#include "connection.h"
#include "event_pooler.h"
#include "async_server.h"
#include "event_manager.h"

namespace {
void handleAcceptEvent(int fd, void* arg, uint8 event, TimeStamp ts);

// move to logic layer.
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

bool createStreamSocket(int family, int* server_fd) {
  int fd = ::socket(family, SOCK_STREAM, 0);
  if (fd == -1) {
    PLOG(WARNING)<< "socket error";
    return false;
  }

  setFdCloExec(fd);
  setFdNonBlock(fd);

  *server_fd = fd;
  return true;
}

bool bindAddr(int listen_fd, sockaddr* addr, int len) {
  int ret = ::bind(listen_fd, addr, len);
  if (ret != 0) {
    PLOG(WARNING)<< "socket bind error: " << listen_fd;
    return false;
  }

  // todo: magic number -> macro def
  ret = ::listen(listen_fd, 4096);
  if (ret != 0) {
    PLOG(WARNING)<< "socket listen error";
    return false;
  }

  return true;
}

}

namespace async {

class AsyncServer::Acceptor {
  public:
    Acceptor(EventManager* ev_mgr, AsyncServer* serv, int server_fd)
        : ev_mgr_(ev_mgr), serv_(serv) {
      server_fd_ = server_fd;
    }
    ~Acceptor() {
      DCHECK(event_ == NULL);  // must call destory first.
      closeWrapper(server_fd_);
    }

    bool init();
    void destory();

    void handleAccept(TimeStamp ts);

  private:
    EventManager* ev_mgr_;
    AsyncServer* serv_;

    int server_fd_;
    scoped_ptr<Event> event_;

    DISALLOW_COPY_AND_ASSIGN(Acceptor);
};

bool AsyncServer::Acceptor::init() {
  if (event_ != nullptr) return false;

  event_.reset(new Event);
  event_->fd = server_fd_;
  event_->arg = this;
  event_->event = EV_READ;
  event_->cb = handleAcceptEvent;

  if (!ev_mgr_->add(event_.get())) {
    event_.reset();
    return false;
  }

  return true;
}

void AsyncServer::Acceptor::destory() {
  if (event_ != nullptr && server_fd_ != INVALID_FD) {
    ev_mgr_->del(*event_);
    event_.reset();
  }
}

void AsyncServer::Acceptor::handleAccept(TimeStamp ts) {
  int fd;
  while (true) {
#if __linux__
    fd = ::accept4(server_fd_, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
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
    serv_->add(conn.get());

    ev_mgr->runInLoop(new ReassignConnectionClosure(ev_mgr, conn.get()));
  }
}

AsyncServer::AsyncServer(EventManager* ev_mgr, int server_fd, uint8 worker)
    : worker_(worker), ev_mgr_(ev_mgr) {
  protocol_ = nullptr;

  DCHECK_NOTNULL(ev_mgr);
  acceptor_.reset(new Acceptor(ev_mgr_, this, server_fd));
}

AsyncServer::~AsyncServer() {
  stop();
}

void AsyncServer::add(Connection* conn) {
  conn->setProtocol(protocol_);
  conn->setData(protocol_->NewConnectionData());
  conn->setCloseClosure(
      ::NewPermanentCallback(this, &AsyncServer::remove, conn));
  ScopedMutex l(&mutex_);
  DCHECK_EQ(map_.count(conn->fileHandle()), 0);
  map_.insert(std::make_pair(conn->fileHandle(), conn));
}

void AsyncServer::remove(Connection* conn) {
  ScopedMutex l(&mutex_);
  auto it = map_.find(conn->fileHandle());
  if (it != map_.end()) {
    map_.erase(it);
    conn->UnRef();
  }
}

void AsyncServer::stop() {
  if (ev_mgr_->inValidThread()) {
    stopInternal(nullptr);
    return;
  }

  SyncEvent ev;
  ev_mgr_->runInLoop(::NewCallback(this, &AsyncServer::stopInternal, &ev));
  ev.Wait();
}

bool AsyncServer::init() {
  if (!acceptor_->init()) {
    acceptor_.reset();
    return false;
  }

  pooler_.reset(new EventPooler(ev_mgr_, worker_));
  if (!pooler_->Init()) {
    acceptor_->destory();
    acceptor_.reset();
    pooler_.reset();
    return false;
  }

  return true;
}

EventManager* AsyncServer::getPoller() {
  DCHECK_NOTNULL(pooler_.get());
  return pooler_->getPoller();
}

void AsyncServer::stopInternal(SyncEvent* ev) {
  ev_mgr_->assertThreadSafe();
  ScopedSyncEvent n(ev);
  if (acceptor_ != nullptr) acceptor_->destory();

  ConnMap tmp;
  {
    ScopedMutex l(&mutex_);
    tmp.swap(map_);
  }
  for (auto it : tmp) {
    Connection* conn = it.second;
    conn->shutDown();
    conn->UnRef();
  }

  if (pooler_ != nullptr) {
    pooler_->stop();
    pooler_.reset();
  }
}

bool createTcpServer(const std::string& ip, uint16 port, int* server_fd) {
  int listen_fd;
  if (!createStreamSocket(AF_INET, &listen_fd)) return false;

  int val = 1;
  int ret = ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &val,
                         sizeof(val));
  if (ret != 0) {
    PLOG(WARNING)<< "setsockopt error";
    closeWrapper(listen_fd);
    return false;
  }

  sockaddr_in addr;
  ::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);
  addr.sin_addr.s_addr = ::inet_addr(ip.c_str());
  if (!bindAddr(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
    closeWrapper(listen_fd);
    return false;
  }

  *server_fd = listen_fd;
  return true;
}

bool createLocalServer(const std::string& path, int* server_fd) {
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX (108)
#endif
  if (path.size() >= UNIX_PATH_MAX) {
    PLOG(WARNING)<<"path is too long: " << path;
    return false;
  }

  int listen_fd;
  if (!createStreamSocket(AF_LOCAL, &listen_fd)) return false;

  struct sockaddr_un addr;
  ::memset(&addr, 0x00, sizeof(addr));
  addr.sun_family = AF_LOCAL;
  ::memcpy(addr.sun_path, path.c_str(), path.size());
  if (!bindAddr(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
    closeWrapper(listen_fd);
    return false;
  }

  *server_fd = listen_fd;
  return true;
}

}

namespace {
void handleAcceptEvent(int fd, void* arg, uint8 event, TimeStamp ts) {
async::AsyncServer::Acceptor* a =
    static_cast<async::AsyncServer::Acceptor*>(arg);
a->handleAccept(ts);
}
}
