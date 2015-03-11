#include "closure_proxy.h"
#include "channel_proxy.h"

#include "timer_list.h"
#include "timer_queue_mac.h"
#include "timer_queue_posix.h"

namespace async {

bool ClosureProxy::Init() {
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
  if (!channel_->init()) {
    channel_.reset();
    timer_queue_.reset();
    return false;
  }

  return true;
}

void ClosureProxy::runInLoop(Closure* cb) {
  channel_->runInLoop(cb);
}

void ClosureProxy::runAt(Closure* cb, TimeStamp ts) {
  timer_queue_->runAt(cb, ts);
}

}
