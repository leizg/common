#include "epoller.h" // included event_manager.h
#include "timer_queue.h"

namespace {

void HandleSignal(int fd, void* arg, uint8 revent, const TimeStamp& time_stamp);

class EventPipe : public io::EventManager::Delegate {
  public:
    explicit EventPipe(io::EventManager* ev_mgr)
        : ev_mgr_(ev_mgr) {
      event_fd_[0] = INVALID_FD;
      event_fd_[1] = INVALID_FD;
    }
    virtual ~EventPipe() {
      if (event_fd_[0] != INVALID_FD) {
        ev_mgr_->Del(event_);
        ::close(event_fd_[0]);
      }

      if (event_fd_[1] != INVALID_FD) {
        ::close(event_fd_[1]);
      }
    }

    void handlePipeRead();

  private:
    io::Event event_;
    io::EventManager* ev_mgr_;

    Mutex mutex_;
    int event_fd_[2];
    std::deque<Closure*> cb_queue_;

    virtual bool Init();
    virtual void runInLoop(Closure* cb);
    virtual void runAt(Closure* cb, const TimeStamp& ts);

    DISALLOW_COPY_AND_ASSIGN(EventPipe);
};

void HandleSignal(int fd, void* arg, uint8 revent,
                  const TimeStamp& time_stamp) {
  EventPipe* p = static_cast<EventPipe*>(arg);
  p->handlePipeRead();
}

bool EventPipe::Init() {
  int ret = ::pipe2(event_fd_, O_NONBLOCK | FD_CLOEXEC);
  if (ret != 0) {
    PLOG(WARNING)<< "pipe error";
    return false;
  }

  event_.fd = event_fd_[0];
  event_.event = EV_READ;
  event_.arg = this;
  event_.cb = HandleSignal;

  return ev_mgr_->Add(&event_);
}

void EventPipe::runInLoop(Closure* cb) {
  if (ev_mgr_->inValidThread()) {
    cb->Run();
    return;
  }

  cb_queue_.push_back(cb);
  uint8 c;
  ::send(event_fd_[1], &c, sizeof(c), 0);
}

void EventPipe::handlePipeRead() {
  uint64 c;
  ::recv(event_fd_[0], &c, sizeof(c), 0);

  std::deque<Closure*> cbs;
  {
    ScopedMutex l(&mutex_);
    cbs.swap(cb_queue_);
  }

  for (uint32 i = 0; i < cbs.size(); ++i) {
    Closure* cb = cbs[i];
    cb->Run();
  }
}

// TODO: later.
void EventPipe::runAt(Closure* cb, const TimeStamp& ts) {
}

}

namespace io {

bool EventManager::Init() {
  delegate_.reset(new EventPipe(this));
  return delegate_->Init();
}

EventManager* CreateEventManager() {
  return new Epoller();
}
}
