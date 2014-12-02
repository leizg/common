#ifndef THREAD_H_
#define THREAD_H_

#include "closure.h"
#include "scoped_ptr.h"
#include "data_types.h"
#include "macro_def.h"

#include <pthread.h>

class Thread {
  public:
    struct Option {
        // static size is 0 means default static size.
        explicit Option(uint32 size = 0)
            : stack_size(size), joinable(true) {
        }
        Option(uint32 size, bool join_able)
            : stack_size(size), joinable(join_able) {
        }

        uint32 stack_size;
        bool joinable;
    };

    explicit Thread(Closure* closure)
        : started_(false), closure_(closure) {
      CHECK_NOTNULL(closure);
      ::memset(&tid_, 0, sizeof(tid_));
    }
    virtual ~Thread() {
      Join();
    }

    virtual void Join() {
      if (option_.joinable && started_) {
        ::pthread_join(tid_, NULL);
        ::memset(&tid_, 0, sizeof(tid_));
      }
      started_ = false;
    }

    bool StartWithOption(const Option& option);
    bool Start() {
      return StartWithOption(Option());
    }

  private:
    pthread_t tid_;
    bool started_;

    Option option_;
    scoped_ptr<Closure> closure_;

    static void* ThreadMain(void* c) {
      Closure* closure = static_cast<Closure*>(c);
      closure->Run();
      return NULL;
    }

    DISALLOW_COPY_AND_ASSIGN(Thread);
};

inline bool Thread::StartWithOption(const Option& option) {
  if (started_) return false;

  ::pthread_attr_t attr;
  int ret = ::pthread_attr_init(&attr);
  if (ret != 0) {
    LOG(WARNING)<< "pthread_attr_init error";
    return false;
  }
  if (option.stack_size != 0) {
    ret = ::pthread_attr_setstacksize(&attr, option.stack_size);
    if (ret != 0) {
      ::pthread_attr_destroy(&attr);
      LOG(WARNING)<< "pthread_attr_setstacksize error";
      return false;
    }
  }
  if (!option.joinable) {
    ret = ::pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (ret != 0) {
      ::pthread_attr_destroy(&attr);
      LOG(WARNING)<< "pthread_attr_setdetachstate error";
      return false;
    }
  }

  ret = ::pthread_create(&tid_, &attr, &Thread::ThreadMain, closure_.get());
  ::pthread_attr_destroy(&attr);
  if (ret != 0) {
    LOG(WARNING)<< "pthread_create error: " << ::strerror(ret);
    return false;
  }
  option_ = option;
  started_ = true;
  return true;
}

class StoppableClosure : public Closure {
  public:
    StoppableClosure(Closure* closure)
        : stop_(false), closure_(closure) {
    }
    virtual ~StoppableClosure() {
      Stop();
    }

    void Stop() {
      stop_ = true;
    }

  private:
    bool stop_;
    scoped_ptr<Closure> closure_;

    void Run() {
      while (!stop_) {
        closure_->Run();
      }
    }

    DISALLOW_COPY_AND_ASSIGN(StoppableClosure);
};

class StoppableThread : public Thread {
  public:
    StoppableThread(Closure* closure)
        : Thread(stoppable_closure_ = new StoppableClosure(closure)) {
    }
    virtual ~StoppableThread() {
      Join();
    }

    virtual void Join() {
      stoppable_closure_->Stop();
      Thread::Join();
    }

  private:
    StoppableClosure* stoppable_closure_;

    DISALLOW_COPY_AND_ASSIGN(StoppableThread);
};

#endif
