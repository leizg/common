#include "event_pooler.h"
#include "event_manager.h"

namespace async {

EventPooler::~EventPooler() {
//  ev_mgr_->assertThreadSafe();
  DCHECK(ev_vec_.empty());
}

bool EventPooler::Init() {
  ev_mgr_->assertThreadSafe();

  for (uint8 i = 0; i < worker_; ++i) {
    EventManager* ev_mgr = CreateEventManager();
    if (!ev_mgr->Init()) {
      stop();
      delete ev_mgr;
      return false;
    }

    ev_mgr->LoopInAnotherThread();
    ev_vec_.push_back(ev_mgr);
  }

  return true;
}

void EventPooler::stop() {
  for (auto it = ev_vec_.begin(); it != ev_vec_.end(); ++it) {
    SyncEvent stop_ev;
    EventManager* ev_mgr = *it;
    ev_mgr->Stop(&stop_ev);
    stop_ev.Wait();
    delete ev_mgr;
  }

  ev_vec_.clear();
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
