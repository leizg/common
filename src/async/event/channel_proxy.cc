#include "event_manager.h"
#include "channel_proxy.h"

namespace async {

ChannelProxy::ChannelProxy(EventManager* ev_mgr)
    : EventPipe(ev_mgr, new PipeDelegate(this)) {
}

void ChannelProxy::runInLoop(Closure* cb) {
  if (ev_mgr_->inValidThread()) {
    cb->Run();
    return;
  }

  {
    ScopedSpinlock l(this);
    cb_queue_.push_back(cb);
  }

  triggerPipe();
}

}

