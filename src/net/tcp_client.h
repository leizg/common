#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include "base/base.h"

namespace net {
class Protocol;
class EventManager;

class Connection;
class OutputObject;

class TcpClient {  // todo: object_saver.h
  public:
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
    // threadsafe, can be called from any thread.
    void Send(OutputObject* io_obj);

  private:
    const std::string ip_;
    uint16 port_;

    EventManager* ev_mgr_;

    Mutex mutex_;
    scoped_ref<Connection> connection_;

    Protocol* protocol_;
    scoped_ptr<Closure> close_closure_;

    void Remove(Connection* conn);

    DISALLOW_COPY_AND_ASSIGN(TcpClient);
};

}

#endif /* TCP_CLIENT_H_ */
