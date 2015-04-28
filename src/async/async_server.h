#pragma once

#include "base/base.h"

namespace async {
class Protocol;
class Connection;

class EventPooler;
class EventManager;

// master + workers.
// master accept new connection, and dispatch it to worker thread.
// thread pool + one event loop per thread.
class AsyncServer {
  public:
    // event_manager must initialized successfully.
    AsyncServer(EventManager* ev_mgr, int server_fd, uint8 worker = 4);
    virtual ~AsyncServer();

    // not threadsafe,
    // must called before initialize server.
    void setProtocol(Protocol* protocol) {
      protocol_ = protocol;
    }

    bool init();
    void stop();

    void add(Connection* conn);
    void remove(Connection* conn);

    // not thread safe.
    // only can be called in loop thread.
    EventManager* getPoller();

  private:
    const uint8 worker_;
    EventManager* ev_mgr_;
    Protocol* protocol_;

    class Acceptor;
    scoped_ptr<Acceptor> acceptor_;
    scoped_ptr<EventPooler> pooler_;

    typedef std::map<int, Connection*> ConnMap;
    Mutex mutex_;
    ConnMap map_;

    void stopInternal(SyncEvent* ev);

    DISALLOW_COPY_AND_ASSIGN(AsyncServer);
};

bool createTcpServer(const std::string& ip, uint16 port, int* server_fd);
bool createLocalServer(const std::string& path, int* server_fd);

}

