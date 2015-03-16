#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include "base/base.h"

namespace io {
class OutputObject;
}

namespace async {
class Protocol;
class EventManager;

class TcpClient {
  public:
    virtual ~TcpClient() {
    }

    static TcpClient* create(EventManager* ev_mgr, const std::string& ip,
                             uint16 port);

    // not thread safe.
    void setProtocol(Protocol* p) {
      protocol_ = p;
    }
    // not thread safe.
    void setCloseClosure(Closure* c) {
      close_closure_.reset(c);
    }

    // please set protocol and closeClosure first.
    // thread safe.
    virtual bool connect(uint32 time_out) = 0;

    // threadsafe, can be called from any thread.
    virtual void send(io::OutputObject* out_obj) = 0;

  protected:
    TcpClient(EventManager* ev_mgr, const std::string& ip, uint16 port)
        : ip_(ip), port_(port), ev_mgr_(ev_mgr), protocol_(NULL) {
      DCHECK(!ip.empty());
      DCHECK_NOTNULL(ev_mgr);
    }

    const std::string ip_;
    uint16 port_;
    EventManager* ev_mgr_;

    Protocol* protocol_;
    scoped_ptr<Closure> close_closure_;

  private:
    DISALLOW_COPY_AND_ASSIGN(TcpClient);
};

}

#endif /* TCP_CLIENT_H_ */
