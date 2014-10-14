#ifndef EVENT_POLLER_H_
#define EVENT_POLLER_H_

#include "base/base.h"

namespace io {
class EventManager;

class EventPoller {
 public:
  EventPoller(EventManager* ev_mgr, uint8 worker);
  ~EventPoller();

  bool Init();

  EventManager* getPoller();

 private:
  EventManager* ev_mgr_;

  typedef std::vector<EventManager*> EvVec;
  EvVec ev_vec_;

  DISALLOW_COPY_AND_ASSIGN(EventPoller);
};

}
#endif /* EVENT_POLLER_H_ */
