#pragma once

#include "base/base.h"

namespace async {
class Protocol;
class EventPooler;
class EventManager;

class Listerner;
class Connection;

// master + workers.
// master accept new connection, and dispatch it to worker thread.
// thread pool + event loop per thread.
class AsyncServer {
  public:
    // event_manager must initialized successfully.
    AsyncServer(EventManager* ev_mgr, uint8 worker, Listerner* listerner)
        : worker_(worker), ev_mgr_(ev_mgr) {
      DCHECK_NOTNULL(ev_mgr);
      DCHECK_NOTNULL(listerner);
      listerner_ = listerner;
    }
    virtual ~AsyncServer();

    bool init();
    void stop();

    void add(Connection* conn);
    void remove(Connection* conn);

    // thread safe.
    EventManager* getPoller();

  private:
    const uint8 worker_;
    EventManager* ev_mgr_;

    Listerner* listerner_;
    scoped_ptr<EventPooler> event_poller_;

    typedef std::map<int, Connection*> ConnMap;
    Mutex mutex_;
    ConnMap map_;

    void stopInternal(SyncEvent* ev);

    DISALLOW_COPY_AND_ASSIGN(AsyncServer);
};
}

