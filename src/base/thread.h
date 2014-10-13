#ifndef THREAD_H_
#define THREAD_H_

#include "closure.h"
#include "scoped_ptr.h"
#include "data_types.h"

#include <pthread.h>

class Thread {
 public:
  Thread(Closure* closure)
      : tid_(INVALID_TID),
        closure_(closure) {
    CHECK_NOTNULL(closure);
  }
  virtual ~Thread() {
    Join();
  }

  virtual void Join() {
    if (tid_ == INVALID_TID) return;
    ::pthread_join(tid_, NULL);
    tid_ = INVALID_TID;
  }
  bool Start() {
    if (tid_ != INVALID_TID) return true;
    int ret = ::pthread_create(&tid_, NULL, &Thread::ThreadMain,
                               closure_.get());
    if (tid_ != 0) {
      LOG(WARNING)<< "pthread_create error: " << ::strerror(ret);
      return false;
    }
    return true;
  }

 private:
  pthread_t tid_;
  scoped_ptr<Closure> closure_;

  static void* ThreadMain(void* c) {
    Closure* closure = static_cast<Closure>(c);
    closure->Run();
    return NULL;
  }

  DISALLOW_COPY_AND_ASSIGN(Thread);
};

class StoppableClosure : public Closure {
 public:
  StoppableClosure(Closure* closure)
      : stop_(false),
        closure_(closure) {
  }
  virtual ~StoppableClosure() {
    Stop();
  }

  void Run() {
    while (!stop_) {
      closure_->Run();
    }
  }

  void Stop() {
    stop_ = true;
  }

 private:
  bool stop_;
  scoped_ptr<Closure> closure_;

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
