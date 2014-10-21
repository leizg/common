#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include "base/base.h"

namespace io {
class Protocol;
class EventManager;

class OutputObject;

class TcpClient {
  public:
    class Connector {
      public:
        virtual ~Connector() {
        }

        // return -1 iff connect fail or timedout.
        virtual int Connect(const std::string& ip, uint16 port,
                            uint32 timeout) const = 0;
    };

    TcpClient(EventManager* ev, const std::string& ip, uint16 port);
    ~TcpClient();

    // not thread safe.
    void SetProtocol(Protocol* p) {
      protocol_ = p;
    }
    // not thread safe.
    void SetCloseClosure(Closure* c) {
      close_closure_.reset(c);
    }

    bool IsConnected() {
      ScopedMutex l(&mutex_);
      return connection_.get() != NULL;
    }

    // please set protocol and closeClosure first.
    bool Connect(uint32 time_out);
    void Send(OutputObject* io_obj);

  private:
    const std::string ip_;
    uint16 port_;
    EventManager* ev_mgr_;

    Mutex mutex_;
    scoped_ptr<Connector> connector_;
    scoped_ref<Connection> connection_;

    Protocol* protocol_;
    scoped_ptr<Closure> close_closure_;

    void Remove(Connection* conn);

    DISALLOW_COPY_AND_ASSIGN(TcpClient);
};

}

#endif /* TCP_CLIENT_H_ */
