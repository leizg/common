#ifndef CLOSURE_PUMP_H_
#define CLOSURE_PUMP_H_

#include "event_manager.h"

namespace io {
namespace detail {

class EventPipe {
  public:
    explicit EventPipe(io::EventManager* ev_mgr)
        : ev_mgr_(ev_mgr) {
      event_fd_[0] = INVALID_FD;
      event_fd_[1] = INVALID_FD;
    }
    virtual ~EventPipe();

    bool Init();
    void runInLoop(Closure* cb);

    void handlePipeRead();

  private:
    io::Event event_;
    io::EventManager* ev_mgr_;

    Mutex mutex_;
    int event_fd_[2];
    std::deque<Closure*> cb_queue_;

    DISALLOW_COPY_AND_ASSIGN(EventPipe);
};

class TimerQueue {
  public:
    class Delegate {
      public:
        virtual ~Delegate() {
        }

        // return expired time
        virtual TimeStamp insert(const TimeStamp time_stamp, Closure* cb) = 0;

        // return false iif release all timers.
        virtual bool fireActivedTimer(const TimeStamp& time_stamp,
                                      TimeStamp* next_expired) = 0;
    };

    explicit TimerQueue(EventManager* ev_mgr)
        : timer_fd_(INVALID_FD), actived_(false), ev_mgr_(ev_mgr) {
      CHECK_NOTNULL(ev_mgr);
    }
    ~TimerQueue();

    bool Init();

    // thread safe.
    // you can call this methord from any thread.
    void runAt(Closure* closure, const TimeStamp& time_stamp);

    // only used for trigger expired events
    // called by event_manager, you shouldn't call this method forever.
    // not threadsafe. must in loop thread.
    void handleRead(const TimeStamp& time_stamp);

  private:
    int timer_fd_;
    bool actived_;

    EventManager* ev_mgr_;
    scoped_ptr<Event> event_;

    TimeStamp expired_time_;
    scoped_ptr<Delegate> delegate_;

    void reset(const TimeStamp time_stamp);
    void runAtInternal(Closure* closure, const TimeStamp time_stamp);

    DISALLOW_COPY_AND_ASSIGN(TimerQueue);
};
}

class ClosurePump : public EventManager::Delegate {
  public:
    explicit ClosurePump(EventManager* ev_mgr)
        : ev_mgr_(ev_mgr) {
      DCHECK_NOTNULL(ev_mgr);
    }
    virtual ~ClosurePump() {
    }

  private:
    virtual bool Init();

    virtual void runInLoop(Closure* cb) {
      ev_pipe_->runInLoop(cb);
    }
    virtual void runAt(Closure* closure, const TimeStamp& time_stamp) {
      timer_queue_->runAt(closure, time_stamp);
    }

    EventManager* ev_mgr_;

    scoped_ptr<detail::EventPipe> ev_pipe_;
    scoped_ptr<detail::TimerQueue> timer_queue_;

    DISALLOW_COPY_AND_ASSIGN(ClosurePump);
};
}
#endif  /*  CLOSURE_PUMP_H_*/
