#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include "base/base.h"

namespace io {
class Timer;
class EventManager;

struct TimerId {
  uint64 timer_id;
  Timer* timer;
};

class TimerQueue {
 public:
  explicit TimerQueue(EventManager* ev_mgr)
      : timer_(NULL),
        ev_mgr_(ev_mgr) {
    CHECK_NOTNULL(ev_mgr);
  }
  ~TimerQueue();

  bool Init();

  void handleRead(const TimeStamp& time_stamp);

  TimerId AddTimer(Closure* closure, uint32 interval, bool repeated);
  void CancelTimer(const TimerId& timer);

 private:
  timer_t timer_;
  EventManager* ev_mgr_;

  typedef std::pair<TimeStamp, Timer*> Entry;
  typedef std::list<Entry> EntryList;
  EntryList entries_;

  typedef std::set<uint64> IdSet;
  IdSet id_set_;

  DISALLOW_COPY_AND_ASSIGN(TimerQueue);
};
}

#endif /* TIMER_QUEUE_H_ */
