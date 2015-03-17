#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include "base/base.h"

namespace async {
struct Event;
class EventManager;

class TimerQueue {
  public:
    class Delegate {
      public:
        virtual ~Delegate() {
        }

        // return expired time
        virtual TimeStamp insert(const TimeStamp& time_stamp, Closure* cb) = 0;

        // return false iif release all timers.
        virtual bool fireActivedTimer(const TimeStamp& time_stamp,
                                      TimeStamp* next_expired) = 0;
    };

    TimerQueue(EventManager* ev_mgr, Delegate* delegate);
    virtual ~TimerQueue();

    virtual bool init();
    virtual void destory();

    // only used for trigger expired events
    // called by event_manager, you shouldn't call this method forever.
    // not threadsafe. must in loop thread.
    virtual void handleRead(TimeStamp time_stamp);
    void runAt(Closure* closure, TimeStamp time_stamp);

  protected:
    bool actived_;

    scoped_ptr<Event> event_;
    EventManager* ev_mgr_;

    TimeStamp expired_time_;
    scoped_ptr<Delegate> delegate_;

    virtual void clearData() = 0;
    virtual void reset(const TimeStamp time_stamp) = 0;

  private:
    void destoryInternal(SyncEvent* ev);
    void runAtInternal(Closure* closure, TimeStamp time_stamp);

    DISALLOW_COPY_AND_ASSIGN(TimerQueue);
};
}
#endif
