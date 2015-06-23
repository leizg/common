#pragma once

#include "event_manager.h"

namespace async {
class TimerQueue;
class ChannelProxy;

class ClosureProxy : public EventManager::ClosureDelegate {
  public:
    explicit ClosureProxy(EventManager* ev_mgr);
    virtual ~ClosureProxy();

  private:
    scoped_ptr<ChannelProxy> channel_;
    scoped_ptr<TimerQueue> timer_queue_;

    virtual bool init();
    virtual void destory();

    virtual void runInLoop(Closure* cb);
    virtual void runAt(Closure* cb, TimeStamp ts);

    DISALLOW_COPY_AND_ASSIGN(ClosureProxy);
};

}

