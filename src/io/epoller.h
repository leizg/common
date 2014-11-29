#ifndef EPOLLER_H_
#define EPOLLER_H_

#ifdef __linux__

#include "event_manager.h"

struct epoll_event;

namespace io {

  class EpollerImpl : public EventManager {
    public:
    EpollerImpl()
    : ep_fd_(INVALID_FD), stop_(true) {
    }
    virtual ~EpollerImpl();

    virtual bool Init();
    virtual void Loop(SyncEvent* start_event = NULL);
    virtual bool LoopInAnotherThread();
    virtual void Stop();

    virtual bool Add(Event* ev);
    virtual void Mod(Event* ev);
    virtual void Del(const Event& ev);

    private:
    int ep_fd_;
    bool stop_;

    // no need lock.
    typedef std::map<int, Event*> EvMap;
    EvMap ev_map_;

    const static uint32 kTriggerNumber = 128;
    std::vector<epoll_event> events_;

    scoped_ptr<StoppableThread> loop_pthread_;

    uint32 convertEvent(uint8 event);

    DISALLOW_COPY_AND_ASSIGN(EpollerImpl);
  };
}

#endif

#endif
