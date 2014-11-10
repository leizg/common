#ifndef EVENT_MANAGER_H_
#define EVENT_MANAGER_H_

#include "include/thread_safe.h"

#define EV_READ   (1<<1)
#define EV_WRITE  (1<<2)
#define EV_ERROR  (1<<3)

namespace io {

struct Event {
    int fd;
    void* arg;
    uint8 event;

    void (*cb)(int fd, void* arg, uint8 revent, const TimeStamp& time_stamp);
};

class EventManager : public ThreadSafe {
  public:
    virtual ~EventManager() {
    }

    virtual bool Init() = 0;
    virtual void Loop(SyncEvent* start_event = NULL) = 0;
    virtual bool LoopInAnotherThread() = 0;
    virtual void Stop() = 0;

    virtual bool Add(Event* ev) = 0;
    virtual void Mod(Event* ev) = 0;
    virtual void Del(const Event& ev) = 0;

    class Delegate {
      public:
        virtual ~Delegate() {
        }

        // called by EventManager::Init.
        virtual bool Init() = 0;

        // must be threadsafe.
        virtual void runInLoop(Closure* cb) = 0;
        virtual void runAt(Closure* cb, const TimeStamp& ts) = 0;
    };
    // these methods are threadsafe,
    // can be called from any thread.
    void runInLoop(Closure* cb) {
      DCHECK_NOTNULL(cb);
      DCHECK_NOTNULL(delegate_.get());
      delegate_->runInLoop(cb);
    }
    void runAt(Closure* cb, const TimeStamp& ts) {
      DCHECK_NOTNULL(cb);
      DCHECK_NOTNULL(delegate_.get());
      delegate_->runAt(cb, ts);
    }

    static EventManager* current();

  protected:
    EventManager() {
    }

  private:
    scoped_ptr<Delegate> delegate_;

    DISALLOW_COPY_AND_ASSIGN(EventManager);
};

EventManager* CreateEventManager();

}

#endif /* EVENT_MANAGER_H_ */
