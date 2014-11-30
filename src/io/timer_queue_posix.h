#ifndef TIMER_QUEUE_POSIX_H_
#define TIMER_QUEUE_POSIX_H_

#if __linux__

#include "timer_queue.h"

namespace io {

class TimerQueuePosix : public TimerQueue {
  public:
    TimerQueuePosix(EventManager* ev_mgr, Delegate* delegate)
        : TimerQueue(ev_mgr, delegate), timer_fd_(INVALID_FD) {
    }
    ~TimerQueuePosix();

  private:
    int timer_fd_;

    virtual bool Init();
    virtual void reset(const TimeStamp time_stamp);

    virtual void clearData();

    DISALLOW_COPY_AND_ASSIGN(TimerQueuePosix);
};
}
#endif  // end for __linux__
#endif  // end for  TIMER_QUEUE_POSIX_H_
