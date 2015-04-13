#ifndef TIMER_LIST_IMPL_H_
#define TIMER_LIST_IMPL_H_

#include "timer_queue.h"

namespace async {

class TimerListImpl : public TimerQueue::Delegate {
  public:
    TimerListImpl() {
    }
    virtual ~TimerListImpl() {
      for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        const Entry& entry = *it;
        delete entry.second;
      }
      entries_.clear();
    }

  private:
    typedef std::pair<TimeStamp, Closure*> Entry;
    typedef std::list<Entry> EntryList;
    EntryList entries_;

    virtual TimeStamp insert(const TimeStamp& time_stamp, Closure* cb);
    virtual bool fireActivedTimer(const TimeStamp& time_stamp,
                                  TimeStamp* next_expired);

    DISALLOW_COPY_AND_ASSIGN(TimerListImpl);
};

}

#endif /* TIMER_LIST_IMPL_H_ */
