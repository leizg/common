#include "kqueue_impl.h"
#include "epoller_impl.h"
#include "event_manager.h"

#include "event_pipe.h"

namespace {
ThreadStorage<async::EventManager> ev_store;

}

namespace async {

bool EventManager::Init() {
  ev_store.set(this);
  return true;
}

void EventManager::Stop(SyncEvent*) {
  ev_store.set(NULL);
}

EventManager* EventManager::current() {
  return ev_store.get();
}

EventManager* CreateEventManager() {
#ifdef __linux__
  return new EpollerImpl();
#else
  return new KqueueImpl();
#endif
}
}
