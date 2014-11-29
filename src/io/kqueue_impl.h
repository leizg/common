#ifndef KQUEUE_IMPL_H_
#define KQUEUE_IMPL_H_

#include "event_manager.h"
#include <sys/event.h>

namespace io {

class KqueueImpl : public EventManager {
  public:
    KqueueImpl()
        : kp_fd_(INVALID_FD), stop_(true) {
    }
    virtual ~KqueueImpl();

    virtual bool Init();
    virtual void Loop(SyncEvent* start_event = NULL);
    virtual bool LoopInAnotherThread();
    virtual void Stop();

    virtual bool Add(Event* ev);
    virtual void Mod(Event* ev);
    virtual void Del(const Event& ev);

  private:
    int kp_fd_;
    bool stop_;

    // no need lock.
    typedef std::map<int, Event*> EvMap;
    EvMap ev_map_;

    scoped_ptr<StoppableThread> loop_pthread_;

    std::vector<struct kevent> events_;
    const static uint32 kTriggerNumber = 128;

    DISALLOW_COPY_AND_ASSIGN(KqueueImpl);
};

}

#endif /* KQUEUE_IMPL_H_ */
