#include "epoller.h" // included event_manager.h

#include "closure_pump.h"

namespace {
ThreadStorage ev_store;
}

namespace io {

bool EventManager::Init() {
  delegate_.reset(new ClosurePump(this));
  if (!delegate_->Init()) {
    delegate_.reset();
    return false;
  }

  ev_store.set(this);
  return true;
}

EventManager* current() {
  return ev_store.get<EventManager*>();
}

EventManager* CreateEventManager() {
  return new Epoller();
}
}
