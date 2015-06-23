#pragma once

#include "event_pipe.h"

namespace async {

class ChannelProxy : public EventPipe, /* private */SpinLock {
  public:
    explicit ChannelProxy(EventManager* ev_mgr);
    virtual ~ChannelProxy() {
      ScopedSpinlock l(this);
      STLClear(&cb_queue_);
    }

    void runInLoop(Closure* cb);
    void release(std::deque<Closure*>* tasks) {
      ScopedSpinlock l(this);
      tasks->swap(cb_queue_);
    }

  private:
    std::deque<Closure*> cb_queue_;

    class PipeDelegate : public EventPipe::Delegate {
      public:
        explicit PipeDelegate(ChannelProxy* p)
            : p_(p) {
          DCHECK_NOTNULL(p);
        }
        virtual ~PipeDelegate() {
        }

      private:
        ChannelProxy* p_;

        virtual void handleEvent(TimeStamp ts) {
          std::deque<Closure*> cbs;
          p_->release(&cbs);

          for (auto cb : cbs) {
            cb->Run();
          }
        }

        DISALLOW_COPY_AND_ASSIGN(PipeDelegate);
    };

    DISALLOW_COPY_AND_ASSIGN(ChannelProxy);
};
}
