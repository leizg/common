#include "closure_proxy.h"
#include "channel_proxy.h"

#include "timer_list_impl.h"
#include "timer_queue_mac.h"
#include "timer_queue_posix.h"

namespace async {

ClosureProxy::ClosureProxy(EventManager* ev_mgr)
    : EventManager::ClosureDelegate(ev_mgr) {
}

ClosureProxy::~ClosureProxy() {
  if (timer_queue_ != nullptr) {
    timer_queue_->destory();
    timer_queue_.reset();
  }
}

bool ClosureProxy::init() {
  channel_.reset(new ChannelProxy(ev_mgr_));
  if (!channel_->init()) {
    channel_.reset();
    return false;
  }

  timer_queue_.reset(new
#ifdef __linux__
                     TimerQueuePosix
#else
                     /* todo: mac os */
#endif
                     (ev_mgr_, new TimerListImpl));
  if (!timer_queue_->init()) {
    channel_.reset();
    timer_queue_.reset();
    return false;
  }

  return true;
}

void ClosureProxy::destory() {
  channel_->destory();
  channel_.reset();

  timer_queue_.reset();
}

void ClosureProxy::runInLoop(Closure* cb) {
  channel_->runInLoop(cb);
}

void ClosureProxy::runAt(Closure* cb, TimeStamp ts) {
  timer_queue_->runAt(cb, ts);
}

}

