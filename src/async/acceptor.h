#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "base/base.h"

namespace aync {
struct Event;
class Protocol;
class TcpServer;

class Acceptor {
  public:
    // not hold ev_mgr.
    Acceptor(EventManager* ev_mgr, TcpServer* serv)
        : listen_fd_(INVALID_FD), serv_(serv), ev_mgr_(ev_mgr) {
      DCHECK_NOTNULL(serv);
      DCHECK_NOTNULL(ev_mgr);
      protocol_ = nullptr;
    }
    ~Acceptor();

    void setProtocol(Protocol* p) {
      DCHECK_NOTNULL(p);
      protocol_ = p;
    }

    void handleAccept();
    bool doBind(const std::string& ip, uint16 port);

  private:
    int listen_fd_;

    TcpServer* serv_;
    EventManager* ev_mgr_;

    Protocol* protocol_;
    scoped_ptr<Event> event_;

    bool createListenFd(const std::string& ip, uint16 port);

    DISALLOW_COPY_AND_ASSIGN(Acceptor);
};

}

#endif /* ACCEPTOR_H_ */
