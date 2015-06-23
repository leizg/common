#pragma once

#include "base/base.h"

namespace async {
class EventManager;

class EventPooler {
  public:
    EventPooler(EventManager* ev_mgr, uint8 worker)
        : worker_(worker), ev_mgr_(ev_mgr), index_(0) {
    }
    ~EventPooler();

    bool Init();
    void stop();

    EventManager* getPoller();

  private:
    uint8 worker_;
    EventManager* ev_mgr_;

    uint8 index_;
    typedef std::vector<EventManager*> EvVec;
    EvVec ev_vec_;

    DISALLOW_COPY_AND_ASSIGN(EventPooler);
};

}
