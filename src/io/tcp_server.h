#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "base/base.h"

namespace io {
class Protocol;
class Connection;
class EventPooler;
class EventManager;

class TcpServer {
 public:
  class Listener {
   public:
    virtual ~Listener() {
    }

    virtual void Accept() = 0;
  };

  // event_manager must initialized successfully.
  TcpServer(EventManager* ev_mgr, const std::string& ip, uint16 port);
  ~TcpServer();

  void setWorker(uint8 worker) {
    worker_ = worker;
  }
  void setProtocol(Protocol* p) {
    protocol_ = p;
  }

  EventManager* getPoller();

  bool Start();
  void Stop();

  void Add(Connection* conn);
  void Remove(Connection* conn);

 private:
  const std::string ip_;
  uint16 port_;
  uint8 worker_;

  EventManager* ev_mgr_;
  Protocol* protocol_;
  scoped_ptr<Listener> listener_;
  scoped_ptr<EventPooler> event_poller_;

  // FIXME: remove this.
  Mutex mutex_;
  typedef std::map<int, Connection*> ConnMap;
  ConnMap conn_map_;

  DISALLOW_COPY_AND_ASSIGN(TcpServer);
};
}

#endif /* TCP_SERVER_H_ */
