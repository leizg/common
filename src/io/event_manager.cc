#include "epoller.h"
#include "timer_queue.h"

namespace {
scoped_ptr<io::Event> signal_ev;

void HandleSignal(int fd, void* arg, uint8 revent,
                  const TimeStamp& time_stamp) {
  io::EventManager* ev_mgr = static_cast<io::EventManager*>(arg);
  ev_mgr->handleClosure();
}
}

namespace io {

EventManager::EventManager()
    : stop_(true) {
  loop_tid_ = ::pthread_self();
  event_fd_[0] = INVALID_FD;
  event_fd_[1] = INVALID_FD;
}

EventManager::~EventManager() {
  if (event_fd_[0] != INVALID_FD) {
    ::close(event_fd_[0]);
  }
  if (event_fd_[1] != INVALID_FD) {
    ::close(event_fd_[1]);
  }
}

bool EventManager::InitPipe() {
  if (signal_ev.get() != NULL) return false;

  timer_queue_.reset(new TimerQueue(this));
  if (!timer_queue_->Init()) {
    return false;
  }

  int ret = ::pipe2(event_fd_, O_NONBLOCK | FD_CLOEXEC);
  if (ret != 0) {
    PLOG(WARNING)<< "pipe error";
    return false;
  }
  signal_ev.reset(new Event);
  signal_ev->fd = event_fd_[0];
  signal_ev->event = EV_READ;
  signal_ev->arg = this;
  signal_ev->cb = HandleSignal;

  return Add(signal_ev.get());
}

void EventManager::runInLoop(Closure* cb) {
  if (inLoopThread()) {
    cb->Run();
    return;
  }

  cb_queue_.push_back(cb);
  uint8 c;
  ::send(event_fd_[1], &c, sizeof(c), 0);
}

void EventManager::handleClosure() {
  uint8 c;
  ::recv(event_fd_[0], &c, sizeof(c), 0);

  std::deque<Closure*> cbs;
  {
    ScopedMutex l(&mutex_);
    cbs.swap(cb_queue_);
  }

  while (!cbs.empty()) {
    Closure* cb = cbs.front();
    cb->Run();
    cbs.pop_front();
  }
}

EventManager* CreateEventManager() {
  return new Epoller();
}
}
