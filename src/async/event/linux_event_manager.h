#pragma once

#ifdef __linux__

#include <sys/epoll.h> // todo: move it to .cc
#include "event_manager.h"

namespace async {

class LinuxEventManager : public EventManager {
  public:
    LinuxEventManager()
        : ep_fd_(INVALID_FD), stop_(true) {
    }
    virtual ~LinuxEventManager() {
      DCHECK(stop_);
      closeWrapper(ep_fd_);
    }

    virtual bool init();
    virtual void loop(SyncEvent* start_event = NULL);
    virtual bool loopInAnotherThread();
    virtual void stop(SyncEvent* ev);

    virtual bool add(Event* ev);
    virtual void mod(Event* ev);
    virtual void del(const Event& ev);

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

    DISALLOW_COPY_AND_ASSIGN (LinuxEventManager);
};
}

#endif // end for __linux__
