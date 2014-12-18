#include "kqueue_impl.h"
#include "epoller_impl.h"
#include "event_manager.h"

#include "event_pipe.h"

namespace {
ThreadStorage<net::EventManager> ev_store;

class ThreadsafePipe : public net::EventManager::ThreadSafeDelegate,
    public net::EventPipe {
  private:
    class PipeDelegate : public net::EventPipe::Delegate {
      public:
        explicit PipeDelegate(ThreadsafePipe* p)
            : p_(p) {
          DCHECK_NOTNULL(p);
        }
        virtual ~PipeDelegate() {
        }

      private:
        ThreadsafePipe* p_;

        virtual void handleEvent();

        DISALLOW_COPY_AND_ASSIGN(PipeDelegate);
    };

  public:
    explicit ThreadsafePipe(io::EventManager* ev_mgr)
        : net::EventPipe(new PipeDelegate(this)), ev_mgr_(ev_mgr) {
      DCHECK_NOTNULL(ev_mgr);
    }
    virtual ~ThreadsafePipe() {
      ScopedSpinlock l(&lock_);
      STLClear(&cb_queue_);
    }

    void release(std::deque<Closure*>* tasks) {
      ScopedSpinlock l(&lock_);
      cb_queue_.swap(*tasks);
    }

  private:
    net::Event event_;
    net::EventManager* ev_mgr_;

    SpinLock lock_;
    std::deque<Closure*> cb_queue_;

    virtual bool Init();
    virtual void runInLoop(Closure* cb);

    DISALLOW_COPY_AND_ASSIGN(ThreadsafePipe);
};

void handleSignal(int fd, void* arg, uint8 revent,
                  const TimeStamp& time_stamp) {
  ThreadsafePipe* p = static_cast<ThreadsafePipe*>(arg);
  p->handlePipeRead();
}

bool ThreadsafePipe::Init() {
  if (!net::EventPipe::initPipe()) return false;

  event_.fd = readablePipeFd();
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

  {
    ScopedSpinlock l(&lock_);
    cb_queue_.push_back(cb);
  }
  triggerPipe();
}

void ThreadsafePipe::PipeDelegate::handleEvent() {
  std::deque<Closure*> cbs;
  p_->release(&cbs);

  for (uint32 i = 0; i < cbs.size(); ++i) {
    Closure* cb = cbs[i];
    cb->Run();
  }
}
}

namespace net {

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
