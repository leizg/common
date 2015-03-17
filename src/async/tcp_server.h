#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "connection.h"
#include "include/object_saver.h"

namespace async {
class Protocol;
class Acceptor;
class EventPooler;
class EventManager;

typedef ThreadSafeObjectSaver<int, Connection, RefCountedObjectMapSaver> ConnTable;

// master + workers.
// master accept new connection, and dispatch it to worker thread.
// thread pool + event loop per thread.
class TcpServer : public MulityTableObjectSaver<int, Connection, ConnTable> {
  public:
    // event_manager must initialized successfully.
    TcpServer(EventManager* ev_mgr, uint8 worker);
    virtual ~TcpServer();

    bool Init();  // set protocol first.
    void setProtocol(Protocol* p) {  // not threadsafe.
      DCHECK_NOTNULL(p);
      DCHECK(protocol_ == NULL);
      protocol_ = p;
    }

    void stop();

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

    void stopInternal(SyncEvent* ev);

    void bindIpInternal(const std::string ip, uint16 port, bool* success,
                        SyncEvent* ev);

    void unBindAllInternal(SyncEvent* ev);
    void unBindIpInternal(const std::string ip, SyncEvent* ev);

    typedef ThreadSafeObjectSaver<std::string, Acceptor, ObjectMapSaver> ListenerMap;
    scoped_ptr<ListenerMap> listeners_;

    DISALLOW_COPY_AND_ASSIGN(TcpServer);
};
}

#endif /* TCP_SERVER_H_ */
