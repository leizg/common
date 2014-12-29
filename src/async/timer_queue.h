#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include "event_manager.h"

namespace aync {

class TimerQueue : public EventManager::TimerDelegate {
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

    TimerQueue(EventManager* ev_mgr, Delegate* delegate)
        : actived_(false), ev_mgr_(ev_mgr), expired_time_(Now()), delegate_(
            delegate) {
      DCHECK_NOTNULL(ev_mgr);
      DCHECK_NOTNULL(delegate);
    }
    virtual ~TimerQueue() {
    }

    // only used for trigger expired events
    // called by event_manager, you shouldn't call this method forever.
    // not threadsafe. must in loop thread.
    virtual void handleRead(const TimeStamp& time_stamp);

  protected:
    bool actived_;

    scoped_ptr<Event> event_;
    EventManager* ev_mgr_;

    TimeStamp expired_time_;
    scoped_ptr<Delegate> delegate_;

    virtual bool Init();
    virtual void clearData() = 0;
    virtual void reset(const TimeStamp time_stamp) = 0;

  private:
    void runAt(Closure* closure, const TimeStamp& time_stamp);
    void runAtInternal(Closure* closure, const TimeStamp time_stamp);

    DISALLOW_COPY_AND_ASSIGN(TimerQueue);
};
}
#endif
