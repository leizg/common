#ifndef EPOLLER_IMPL_H_
#define EPOLLER_IMPL_H_

#ifdef __linux__

#include <sys/epoll.h> // todo: move it to .cc
#include "event_manager.h"

namespace async {

class EpollerImpl : public EventManager {
  public:
    EpollerImpl()
        : ep_fd_(INVALID_FD), stop_(true) {
    }
    virtual ~EpollerImpl() {
      DCHECK(stop_);
      closeWrapper(ep_fd_);
    }

    virtual bool Init();
    virtual void Loop(SyncEvent* start_event = NULL);
    virtual bool LoopInAnotherThread();
    virtual void Stop(SyncEvent* ev);

    virtual bool Add(Event* ev);
    virtual void Mod(Event* ev);
    virtual void Del(const Event& ev);

  private:
    int ep_fd_;
    bool stop_;

    typedef std::map<int, Event*> EvMap;
    EvMap ev_map_;

    const static uint32 kTriggerNumber = 128;
    std::vector<epoll_event> events_;

    scoped_ptr<StoppableThread> loop_pthread_;

    void stopInternal(SyncEvent* ev);
    uint32 convertEvent(uint8 event);

    DISALLOW_COPY_AND_ASSIGN(EpollerImpl);
};
}
#endif // end for __linux__
#endif // end for EPOLLER_IMPL_H_
