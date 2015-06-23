#include "mac_event_manager.h"
#include "linux_event_manager.h"

#include "event_pipe.h"
#include "event_manager.h"

namespace {
ThreadStorage<async::EventManager> ev_store;

}

namespace async {

bool EventManager::init() {
  if (ev_store.get() == nullptr) {
    ev_store.set(this);
    return true;
  }

  LOG(WARNING)<< "already exist";
  return false;
}

void EventManager::stop(SyncEvent*) {
  ev_store.set(nullptr);
}

EventManager* EventManager::current() {
  return ev_store.get();
}

EventManager* CreateEventManager() {
#ifdef __linux__
  return new LinuxEventManager();
#else
  return new KqueueImpl();
#endif
}
}
