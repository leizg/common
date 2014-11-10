#include "epoller.h" // included event_manager.h
#include "closure_pump.h"


namespace io {

bool EventManager::Init() {
  delegate_.reset(new ClosurePump(this));
  return delegate_->Init();
}

EventManager* CreateEventManager() {
  return new Epoller();
}
}
