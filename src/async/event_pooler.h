#ifndef EVENT_POOLER_H_
#define EVENT_POOLER_H_

#include "base/base.h"

namespace aync {
class EventManager;

class EventPooler {
  public:
    EventPooler(EventManager* ev_mgr, uint8 worker)
        : worker_(worker), ev_mgr_(ev_mgr), index_(0) {
    }
    ~EventPooler();

    bool Init();

    EventManager* getPoller();

  private:
    uint8 worker_;
    EventManager* ev_mgr_;

    Mutex mutex_;
    uint8 index_;
    typedef std::vector<EventManager*> EvVec;
    EvVec ev_vec_;

    DISALLOW_COPY_AND_ASSIGN(EventPooler);
};

}
#endif /* EVENT_POOLER_H_ */
