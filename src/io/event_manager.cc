#include "kqueue_impl.h"
#include "epoller_impl.h"
#include "event_manager.h"

#include "event_pipe.h"

namespace {
ThreadStorage<io::EventManager> ev_store;

class ThreadsafePipe : public io::EventManager::ThreadSafeDelegate,
    public io::EventPipe {
  private:
    class PipeDelegate : public Delegate {
      public:
        PipeDelegate(Mutex* mutex, std::deque<Closure*>* cb_queue)
            : mutex_(mutex), cb_queue_(cb_queue) {
          DCHECK_NOTNULL(mutex);
          DCHECK_NOTNULL(cb_queue);
        }
        virtual ~PipeDelegate() {
        }

      private:
        Mutex* mutex_;
        std::deque<Closure*>* cb_queue_;

        virtual void handlevent();

        DISALLOW_COPY_AND_ASSIGN(PipeDelegate);
    };

  public:
    explicit ThreadsafePipe(io::EventManager* ev_mgr)
        : io::EventPipe(new PipeDelegate(&mutex_, &cb_queue_)), ev_mgr_(ev_mgr) {
      DCHECK_NOTNULL(ev_mgr);
    }
    virtual ~ThreadsafePipe() {
      ScopedMutex l(&mutex_);
      STLClear(&cb_queue_);
    }

  private:
    io::Event event_;
    io::EventManager* ev_mgr_;

    Mutex mutex_;
    std::deque<Closure*> cb_queue_;

    virtual bool Init();
    virtual void runInLoop(Closure* cb);

    DISALLOW_COPY_AND_ASSIGN(ThreadsafePipe);
};

void handleSignal(int fd, void* arg, uint8 revent,
                  const TimeStamp& time_stamp) {
  io::EventPipe* p = static_cast<io::EventPipe*>(arg);
  p->handlePipeRead();
}

bool ThreadsafePipe::Init() {
  if (!io::EventPipe::initPipe()) return false;

  event_.fd = readPipeFd();
  event_.event = EV_READ;
  event_.arg = this;
  event_.cb = handleSignal;

  return ev_mgr_->Add(&event_);
}

void ThreadsafePipe::runInLoop(Closure* cb) {
  if (ev_mgr_->inValidThread()) {
    cb->Run();
    return;
  }

  cb_queue_.push_back(cb);
  triggerPipe();
}

void ThreadsafePipe::PipeDelegate::handlevent() {
  std::deque<Closure*> cbs;
  {
    ScopedMutex l(mutex_);
    cbs.swap(*cb_queue_);
  }

  for (uint32 i = 0; i < cbs.size(); ++i) {
    Closure* cb = cbs[i];
    cb->Run();
  }
}
}

namespace io {

bool EventManager::Init() {
  thread_safe_delegate_.reset(new ThreadsafePipe(this));
  if (!thread_safe_delegate_->Init()) {
    thread_safe_delegate_.reset();
    return false;
  }

  ev_store.set(this);
  return true;
}

void EventManager::Stop() {
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
