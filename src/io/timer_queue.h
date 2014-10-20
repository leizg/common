#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include "base/base.h"

namespace io {
class Timer;
struct Event;
class EventManager;

struct TimerId {
    uint64 timer_id;
    Timer* timer;
};

class TimerQueue {
  public:
    explicit TimerQueue(EventManager* ev_mgr)
        : timer_fd_(INVALID_FD), ev_mgr_(ev_mgr) {
      CHECK_NOTNULL(ev_mgr);
    }
    ~TimerQueue();

    bool Init();

    // thread safe.
    // you can call this methord from any thread.
    TimerId AddTimer(Closure* closure, uint32 interval, bool repeated);

    // thread safe.
    // you can call this methord from any thread.
    void CancelTimer(const TimerId& timer);

    // only used for trigger expired events
    // called by event_manager, you shouldn't call this method forever.
    // not threadsafe. must in loop thread.
    void handleRead(const TimeStamp& time_stamp);

  private:
    int timer_fd_;

    EventManager* ev_mgr_;
    scoped_ptr<Event> event_;

    typedef std::pair<TimeStamp, Timer*> Entry;
    typedef std::list<Entry> EntryList;
    EntryList entries_;

    typedef std::set<uint64> IdSet;
    IdSet id_set_;

    // return true if first changed.
    bool insert(Timer* timer);
    void Reset();
    void ReleaseActiveTimer(uint32 trigger_count,
                            std::vector<Timer*>* timer_vec);

    DISALLOW_COPY_AND_ASSIGN(TimerQueue);
};
}

#endif /* TIMER_QUEUE_H_ */
