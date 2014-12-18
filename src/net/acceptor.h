#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "tcp_server.h"

namespace net {
struct Event;

class Acceptor : public TcpServer::Listener {
  public:
    // not hold ev_mgr.
    Acceptor(EventManager* ev_mgr, TcpServer* serv)
        : listen_fd_(INVALID_FD), serv_(serv), ev_mgr_(ev_mgr), protocol_(NULL) {
      DCHECK_NOTNULL(serv);
      DCHECK_NOTNULL(ev_mgr);
      port_ = 0;
    }
    virtual ~Acceptor();

    void setProtocol(Protocol* p) {
      protocol_ = p;
    }

    void handleAccept();
    virtual bool doBind(const std::string& ip, uint16 port);

  private:
    int listen_fd_;

    std::string ip_;
    uint16 port_;

    TcpServer* serv_;
    EventManager* ev_mgr_;

    Protocol* protocol_;
    scoped_ptr<Event> event_;

    bool CreateListenFd(const std::string& ip, uint16 port);

    DISALLOW_COPY_AND_ASSIGN(Acceptor);
};

}

#endif /* ACCEPTOR_H_ */
