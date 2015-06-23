#pragma once

#if __linux__

#include "timer_queue.h"

namespace async {

class TimerQueuePosix : public TimerQueue {
  public:
    TimerQueuePosix(EventManager* ev_mgr, Delegate* delegate)
        : TimerQueue(ev_mgr, delegate) {
      timer_fd_ = INVALID_FD;
    }
    virtual ~TimerQueuePosix();

  private:
    int timer_fd_;

    virtual bool init();
    virtual void destory();

    virtual void clearData();
    virtual void reset(TimeStamp time_stamp);

    DISALLOW_COPY_AND_ASSIGN (TimerQueuePosix);
};

}
#endif  // end for __linux__

