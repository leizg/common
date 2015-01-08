#include "event_pooler.h"
#include "event_manager.h"

namespace aync {

EventPooler::~EventPooler() {
  ev_mgr_->assertThreadSafe();
  STLClear(&ev_vec_);
}

bool EventPooler::Init() {
  ev_mgr_->assertThreadSafe();

  for (uint8 i = 0; i < worker_; ++i) {
    EventManager* ev_mgr = CreateEventManager();
    if (!ev_mgr->Init()) {
      STLClear(&ev_vec_);
      return false;
    }

    ev_mgr->LoopInAnotherThread();
    ev_vec_.push_back(ev_mgr);
  }

  return true;
}

EventManager* EventPooler::getPoller() {
  ev_mgr_->assertThreadSafe();
  if (ev_vec_.empty()) return ev_mgr_;

  if (index_ == ev_vec_.size()) {
    index_ = 0;
    return ev_mgr_;
  }

  DCHECK_LT(index_, ev_vec_.size());
  return ev_vec_[index_++];
}

}
