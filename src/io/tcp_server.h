#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "include/object_saver.h"

namespace io {
class Protocol;
class Connection;

class EventPooler;
class EventManager;

// master + workers.
// master accept new connection, and dispatch it to worker.
// thread pool + event loop per thread.
class TcpServer {
  public:
    class Listener {
      public:
        virtual ~Listener() {
        }

        virtual bool doBind(const std::string& ip, uint16 port) = 0;
    };

    // event_manager must initialized successfully.
    TcpServer(EventManager* ev_mgr, uint8 worker);
    ~TcpServer();

    bool Init();
    void setProtocol(Protocol* p) {  // not threadsafe.
      protocol_ = p;
    }

    // thread safe.
    EventManager* getPoller();

    // threadsafe, can be called from any thread.
    bool bindIp(const std::string& ip, uint16 port);
    void unBindIp(const std::string& ip);
    void unBindAll();

    void Add(Connection* conn);
    void Remove(Connection* conn);

  private:
    uint8 worker_;
    Protocol* protocol_;

    EventManager* ev_mgr_;
    scoped_ptr<EventPooler> event_poller_;

    Mutex mutex_;
    std::map<std::string, Listener*> listeners_;
    ObjectMapSaver<int, Connection> saver_;

    DISALLOW_COPY_AND_ASSIGN(TcpServer);
};
}

#endif /* TCP_SERVER_H_ */
