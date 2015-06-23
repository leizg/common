#pragma once

#include "include/thread_safe.h"

#define EV_READ   (1<<0)
#define EV_WRITE  (1<<1)
#define EV_ERROR  (1<<2)

namespace async {

struct Event {
    int fd;
    void* arg;
    uint8 event;

    void (*cb)(int fd, void* arg, uint8 revent, TimeStamp time_stamp);
};

class EventManager : public ThreadSafe {
  public:
    virtual ~EventManager() {
    }

    virtual bool init() = 0;
    virtual void loop(SyncEvent* start_event = nullptr) = 0;
    virtual bool loopInAnotherThread() = 0;
    virtual void stop(SyncEvent* stop_event) = 0;

    virtual bool add(Event* ev) = 0;
    virtual void mod(Event* ev) = 0;
    virtual void del(const Event& ev) = 0;

    class ClosureDelegate {
      public:
        virtual ~ClosureDelegate() {
        }

        // called after EventManager::Init.
        virtual bool init() = 0;
        virtual void destory() = 0;

        virtual void runInLoop(Closure* cb) = 0;
        virtual void runAt(Closure* cb, TimeStamp ts) = 0;

      protected:
        explicit ClosureDelegate(EventManager* ev_mgr)
            : ev_mgr_(ev_mgr) {
          DCHECK_NOTNULL(ev_mgr);
        }

        EventManager* ev_mgr_;

      private:
        DISALLOW_COPY_AND_ASSIGN (ClosureDelegate);
    };

    // these methods are threadsafe,
    // can be called from any thread.
    void runInLoop(Closure* cb) {
      DCHECK_NOTNULL(cb);
      cb_delegate_->runInLoop(cb);
    }
    void runAt(Closure* cb, TimeStamp ts) {
      DCHECK_NOTNULL(cb);
      cb_delegate_->runAt(cb, ts);
    }

    static EventManager* current();

  protected:
    EventManager() {
    }

    scoped_ptr<ClosureDelegate> cb_delegate_;

  private:
    DISALLOW_COPY_AND_ASSIGN (EventManager);
};

EventManager* CreateEventManager();

}

