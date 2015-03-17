#ifndef EVENT_MANAGER_H_
#define EVENT_MANAGER_H_

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

    virtual bool Init() = 0;
    virtual void Loop(SyncEvent* start_event = NULL) = 0;
    virtual bool LoopInAnotherThread() = 0;
    virtual void Stop(SyncEvent* stop_event) = 0;

    virtual bool Add(Event* ev) = 0;
    virtual void Mod(Event* ev) = 0;
    virtual void Del(const Event& ev) = 0;

    class ClosureDelegate {
      public:
        virtual ~ClosureDelegate() {
        }

        // called after EventManager::Init.
        virtual bool Init() = 0;
        virtual void destory() = 0;

        virtual void runInLoop(Closure* cb) = 0;
        virtual void runAt(Closure* cb, TimeStamp ts) = 0;

      protected:
        explicit ClosureDelegate(EventManager* ev_mgr)
            : ev_mgr_(ev_mgr) {
          DCHECK_NOTNULL(ev_mgr);
        }

        EventManager* ev_mgr_;
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
    DISALLOW_COPY_AND_ASSIGN(EventManager);
};

EventManager* CreateEventManager();

}

#endif /* EVENT_MANAGER_H_ */
