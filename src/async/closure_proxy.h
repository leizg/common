#ifndef CLOSURE_PROXY_H_
#define CLOSURE_PROXY_H_

#include "event_manager.h"

namespace async {
class TimerQueue;
class ChannelProxy;

class ClosureProxy : public EventManager::ClosureDelegate {
  public:
    explicit ClosureProxy(EventManager* ev_mgr)
        : EventManager::ClosureDelegate(ev_mgr) {
    }
    virtual ~ClosureProxy() {
    }

  private:
    scoped_ptr<ChannelProxy> channel_;
    scoped_ptr<TimerQueue> timer_queue_;

    virtual bool Init();

    virtual void runInLoop(Closure* cb);
    virtual void runAt(Closure* cb, TimeStamp ts);

    DISALLOW_COPY_AND_ASSIGN(ClosureProxy);
};

}

#endif /* CLOSURE_PROXY_H_ */
