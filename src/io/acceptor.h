#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "tcp_server.h"

namespace io {
struct Event;

class Acceptor : public TcpServer::Listener {
 public:
  // not hold ev_mgr.
  Acceptor(EventManager* ev_mgr, TcpServer* serv)
      : listen_fd_(INVALID_FD),
        serv_(serv),
        ev_mgr_(ev_mgr),
        protocol_(NULL) {
    CHECK_NOTNULL(serv);
    CHECK_NOTNULL(ev_mgr);
  }
  virtual ~Acceptor();

  void setProtocol(Protocol* p) {
    protocol_ = p;
  }

  bool Init(const std::string& ip, uint16 port);

  void Accept();

 private:
  int listen_fd_;

  TcpServer* serv_;
  EventManager* ev_mgr_;

  Protocol* protocol_;
  scoped_ptr<Event> event_;

  bool CreateListenFd(const std::string& ip, uint16 port);

  DISALLOW_COPY_AND_ASSIGN(Acceptor);
};

}

#endif /* ACCEPTOR_H_ */
