#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "connection.h"
#include "include/object_saver.h"

namespace aync {
class Protocol;
class Connection;

class Acceptor;
class EventPooler;
class EventManager;

typedef ThreadSafeObjectSaver<int, Connection, RefCountedObjectMapSaver> ConnTable;

// master + workers.
// master accept new connection, and dispatch it to worker.
// thread pool + event loop per thread.
class TcpServer : public MulityTableObjectSaver<int, Connection, ConnTable> {
  public:
    // event_manager must initialized successfully.
    TcpServer(EventManager* ev_mgr, uint8 worker);
    ~TcpServer();

    bool Init();  // set protocol first.
    void setProtocol(Protocol* p) {  // not threadsafe.
      DCHECK_NOTNULL(p);
      protocol_ = p;
    }

    // thread safe.
    EventManager* getPoller();

    // threadsafe, can be called from any thread.
    bool bindIp(const std::string& ip, uint16 port);

    void unBindAll();
    void unBindIp(const std::string& ip);

  private:
    const uint8 worker_;
    Protocol* protocol_;

    EventManager* ev_mgr_;
    scoped_ptr<EventPooler> event_poller_;

    typedef ThreadSafeObjectSaver<std::string, Acceptor, ObjectMapSaver> ListenerMap;
    Mutex mutex_;
    scoped_ptr<ListenerMap> listeners_;

    DISALLOW_COPY_AND_ASSIGN(TcpServer);
};
}

#endif /* TCP_SERVER_H_ */
