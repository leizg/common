#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "base/base.h"

namespace async {
struct Event;
class EventManager;

class Protocol;
class AsyncServer;

class Listerner {
  public:
    virtual ~Listerner() {
    }

    void setProtocol(Protocol* p) {  // not threadsafe.
      DCHECK_NOTNULL(p);
      DCHECK(protocol_ == NULL);
      protocol_ = p;
    }

    virtual void stop() = 0;

  protected:
    Listerner(EventManager* ev_mgr, AsyncServer* serv)
        : ev_mgr_(ev_mgr), serv_(serv) {
      DCHECK_NOTNULL(serv);
      DCHECK_NOTNULL(ev_mgr);
      protocol_ = nullptr;
    }

    EventManager* ev_mgr_;
    AsyncServer* serv_;

    Protocol* protocol_;

  private:
    DISALLOW_COPY_AND_ASSIGN(Listerner);
};

class TcpAcceptor : public Listerner {
  public:
    // not hold ev_mgr.
    TcpAcceptor(EventManager* ev_mgr, AsyncServer* serv)
        : Listerner(ev_mgr, serv), protocol_(nullptr) {
    }
    ~TcpAcceptor();

    // threadsafe, can be called from any thread.
    bool bindIp(const std::string& ip, uint16 port);

    void unBindAll();
    void unBindIp(const std::string& ip);

  private:
    class Impl : public Listerner {
      public:
        Impl(EventManager* ev_mgr, const std::string& ip, uint16 port)
            : ip_(ip), port_(port), listen_fd_(INVALID_FD) {
          ev_mgr_ = ev_mgr;
          protocol_ = nullptr;
        }
        virtual ~Impl();

        void setProtocol(Protocol* p) {  // not threadsafe.
          DCHECK_NOTNULL(p);
          DCHECK(protocol_ == NULL);
          protocol_ = p;
        }

        bool init();

        const std::string& bindingIp() const {
          return ip_;
        }
        const int bindingFd() const {
          return listen_fd_;
        }

        void doAccept(TimeStamp ts);

      private:
        const std::string ip_;
        uint16 port_;

        int listen_fd_;
        EventManager* ev_mgr_;
        scoped_ptr<Event> event_;

        Protocol* protocol_;

        virtual void stop();

        DISALLOW_COPY_AND_ASSIGN(Impl);
    };

    typedef std::map<const std::string, Impl*> Map;
    RwLock rw_lock_;
    Map map_;

    virtual void stop();

    DISALLOW_COPY_AND_ASSIGN(TcpAcceptor);
};
}
#endif /* ACCEPTOR_H_ */
