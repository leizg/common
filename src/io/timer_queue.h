#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include "base/base.h"

namespace io {
struct Event;
class EventManager;

class TimerQueue {
  public:
    class Delegate {
      public:
        virtual ~Delegate() {
        }

        // return expired time
        virtual TimeStamp insert(const TimeStamp time_stamp, Closure* cb) = 0;

        // return false iif release all timers.
        virtual bool fireActivedTimer(const TimeStamp& time_stamp,
                                      TimeStamp* next_expired) = 0;
    };

    explicit TimerQueue(EventManager* ev_mgr)
        : timer_fd_(INVALID_FD), actived_(false), ev_mgr_(ev_mgr) {
      CHECK_NOTNULL(ev_mgr);
    }
    ~TimerQueue();

    bool Init();

    // thread safe.
    // you can call this methord from any thread.
    void runAt(Closure* closure, const TimeStamp& time_stamp);

    // only used for trigger expired events
    // called by event_manager, you shouldn't call this method forever.
    // not threadsafe. must in loop thread.
    void handleRead(const TimeStamp& time_stamp);

  private:
    int timer_fd_;
    bool actived_;

    EventManager* ev_mgr_;
    scoped_ptr<Event> event_;

    TimeStamp expired_time_;
    scoped_ptr<Delegate> delegate_;

    void reset(const TimeStamp time_stamp);
    void runAtInternal(Closure* closure, const TimeStamp time_stamp);

    DISALLOW_COPY_AND_ASSIGN(TimerQueue);
};
}

#endif /* TIMER_QUEUE_H_ */
