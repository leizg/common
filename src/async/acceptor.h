#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "base/base.h"

namespace async {
struct Event;
class EventManager;

class Protocol;
class AsyncServer;

class Addr {
  public:
    virtual ~Addr() {
    }

    enum TYPE {
      LOCAL_ADDR, TCP_ADDR,
    };

    TYPE type() const {
      return type_;
    }

    struct sockaddr* addr() const {
      return reinterpret_cast<sockaddr*>(&addr_);
    }

    static Addr* copyFrom(const Addr& rhs);
    static Addr* createAddr(const std::string& path);
    static Addr* createAddr(const std::string& ip, uint16 port);

    virtual const std::string toString() const = 0;

  protected:
    explicit Addr(TYPE type)
        : type_(type) {
      ::memset(&addr_, 0, sizeof(addr_));
    }

    sockaddr_storage addr_;

  private:
    TYPE type_;

    DISALLOW_COPY_AND_ASSIGN(Addr);
};

class TcpAddr : public Addr {
  public:
    TcpAddr(const std::string& ip, uint16 port)
        : Addr(TCP_ADDR), ip_(ip), port_(port) {
    }
    virtual ~TcpAddr() {
    }

    const std::string& ip() {
      return ip_;
    }
    uint16 port() const {
      return port_;
    }

    virtual const std::string toString() const;

  private:
    std::string ip_;
    uint16 port_;

    DISALLOW_COPY_AND_ASSIGN(TcpAddr);
};

class LocalAddr : public Addr {
  public:
    explicit LocalAddr(const std::string& path)
        : Addr(LOCAL_ADDR), path_(path) {
    }
    virtual ~LocalAddr() {
    }

    const std::string& path() const {
      return path_;
    }

    virtual const std::string toString() const {
      return path_;
    }

  private:
    std::string path_;

    DISALLOW_COPY_AND_ASSIGN(LocalAddr);
};

class AddrBinder {
  public:
    AddrBinder(EventManager* ev_mgr, const Addr& addr)
        : listen_fd_(INVALID_FD), ev_mgr_(ev_mgr) {
      DCHECK_NOTNULL(ev_mgr);
    }
    virtual ~AddrBinder() {
    }

    int fd() const {
      return listen_fd_;
    }
    const Addr& addr() const {
      return *addr_;
    }

    bool bind();
    void unbind();

  protected:
    int listen_fd_;
    EventManager* ev_mgr_;

    scoped_ptr<Addr> addr_;
    scoped_ptr<Event> event_;

  private:
    DISALLOW_COPY_AND_ASSIGN(AddrBinder);
};

class Acceptor {
  public:
    // not hold ev_mgr.
    // ev_mgr have been initialized successfully.
    Acceptor(EventManager* ev_mgr, AsyncServer* serv)
        : ev_mgr_(ev_mgr), serv_(serv) {
    }
    ~Acceptor();

    // threadsafe, can be called from any thread.
    bool bind(const std::string& path);
    bool bind(const std::string& ip, uint16 port);

    void unBindAll();
    void unBind(const std::string& path);
    void unBind(const std::string& ip, uint16 port);

  private:
    EventManager* ev_mgr_;
    AsyncServer* serv_;

    class Impl : public AddrBinder {
      public:
        Impl(EventManager* ev_mgr, const std::string& ip, uint16 port)
            : ip_(ip), port_(port), listen_fd_(INVALID_FD) {
          ev_mgr_ = ev_mgr;
        }
        virtual ~Impl();

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

        virtual void stop();

        DISALLOW_COPY_AND_ASSIGN(Impl);
    };

    typedef std::map<const std::string, Impl*> Map;
    Map map_;

    DISALLOW_COPY_AND_ASSIGN(Acceptor);
};
}
#endif /* ACCEPTOR_H_ */
