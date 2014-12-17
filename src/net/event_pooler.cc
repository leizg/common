#include "event_pooler.h"
#include "event_manager.h"

namespace io {

EventPooler::~EventPooler() {
  ScopedMutex l(&mutex_);
  STLClear(&ev_vec_);
}

bool EventPooler::Init() {
  ScopedMutex l(&mutex_);

  for (uint8 i = 0; i < worker_; ++i) {
    EventManager* ev_mgr = CreateEventManager();
    if (!ev_mgr->Init()) {
      LOG(WARNING)<< "evmgr init error";
      STLClear(&ev_vec_);
      return false;
    }

    ev_mgr->LoopInAnotherThread();
    ev_vec_.push_back(ev_mgr);
  }

  return true;
}

EventManager* EventPooler::getPoller() {
  ScopedMutex l(&mutex_);
  if (ev_vec_.empty()) return ev_mgr_;

  if (index_ == ev_vec_.size()) {
    index_ = 0;
    return ev_mgr_;
  }

  CHECK_LT(index_, ev_vec_.size());
  return ev_vec_[index_++];
}

}
