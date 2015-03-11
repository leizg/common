#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "base/base.h"

namespace async {
struct Event;
class EventManager;

class Protocol;
class TcpServer;

class Acceptor {
  public:
    // not hold ev_mgr.
    Acceptor(EventManager* ev_mgr, TcpServer* serv)
        : listen_fd_(INVALID_FD), protocol_(nullptr) {
      DCHECK_NOTNULL(serv);
      DCHECK_NOTNULL(ev_mgr);
      serv_ = serv;
      ev_mgr_ = ev_mgr;
    }
    ~Acceptor();

    void setProtocol(Protocol* p) {
      DCHECK_NOTNULL(p);
      DCHECK(protocol_ == NULL);
      protocol_ = p;
    }

    void handleAccept(TimeStamp ts);
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
