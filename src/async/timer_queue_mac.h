#ifndef TIMER_QUEUE_MAC_H_
#define TIMER_QUEUE_MAC_H_

#ifdef __APPLE__
#include "timer_queue.h"

namespace aync {
class KqueueImpl;

class TimerQueueMac : public TimerQueue {
  public:
    TimerQueueMac(KqueueImpl* kq, Delegate* delegate);
    virtual ~TimerQueueMac() {
      closeWrapper(dummy_);
    }

  private:
    KqueueImpl* kq_;

    int dummy_;
    const static std::string dummy_path_;

    virtual bool Init();
    virtual void reset(const TimeStamp time_stamp);

    virtual void clearData() {
    }

    DISALLOW_COPY_AND_ASSIGN(TimerQueueMac);
};
}
#endif  // end for __APPLE__
#endif  /* TIMER_QUEUE_MAC_H_ */
