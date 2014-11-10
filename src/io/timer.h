#ifndef TIMER_H_
#define TIMER_H_

#include "event_manager.h"

namespace io {

namespace detail {
template<bool is_repeat>
class TimerAbstruct {
  public:
    virtual ~TimerAbstruct() {
    }

    bool isRunning() const {
      if (delay_task_ == NULL) return false;
      return delay_task_->timer != NULL;
    }

    void start() {
      ev_mgr_->assertThreadSafe();
      if (!isRunning()) {
        delay_task_ = new TimerTask(ev_mgr_);
        delay_task_->timer = this;
        ev_mgr_->runAt(delay_task_, TimeStamp::afterMicroSeconds(expired_));
      }
    }
    void stop() {
      ev_mgr_->assertThreadSafe();
      if (isRunning()) {
        delay_task_->timer = NULL;
        delay_task_ = NULL;
      }
    }

    void reset() {
      stop();
      start();
    }

  protected:
    TimerAbstruct(uint64 micro_secs, Closure* closure)
        : ev_mgr_(NULL), expired_(micro_secs), closure_(closure), delay_task_(
        NULL) {
      ev_mgr_ = EventManager::current();
      DCHECK_NOTNULL(ev_mgr_);
      DCHECK_NOTNULL(closure);
    }
    TimerAbstruct(uint64 micro_secs, EventManager* ev_mgr, Closure* closure)
        : ev_mgr_(ev_mgr), expired_(micro_secs), closure_(closure), delay_task_(
        NULL) {
      DCHECK_NOTNULL(ev_mgr);
      DCHECK_NOTNULL(closure);
    }

    class TimerTask : public Closure {
      public:
        TimerTask(EventManager* ev_mgr)
            : timer(NULL), ev_mgr_(ev_mgr) {
        }
        virtual ~TimerTask() {
          if (timer != NULL) {
            timer->stop();
          }
        }

        TimerAbstruct* timer;

      protected:
        void Run() {
          ev_mgr_->assertThreadSafe();
          if (timer != NULL) {  // not cancel.
            if (timer->closure_ != NULL) {
              timer->closure_->Run();
            }
            timer->delay_task_ = NULL;
            if (is_repeat) {
              timer->reset();
            }
          }
          delete this;
        }

      private:
        EventManager* ev_mgr_;

        DISALLOW_COPY_AND_ASSIGN(TimerTask);
    };

    EventManager* ev_mgr_;
    uint64 expired_;

    scoped_ptr<Closure> closure_;
    TimerTask* delay_task_;

  private:
    DISALLOW_COPY_AND_ASSIGN(TimerAbstruct);
};
}

// not thread safe.
class OneshotTimer : public detail::TimerAbstruct<false> {
  public:
    OneshotTimer(uint64 micro_secs, Closure* closure)
        : detail::TimerAbstruct<false>(micro_secs, closure) {
    }
    OneshotTimer(uint64 micro_secs, EventManager* ev_mgr, Closure* closure)
        : detail::TimerAbstruct<false>(micro_secs, ev_mgr, closure) {
    }
    virtual ~OneshotTimer();

  private:
    DISALLOW_COPY_AND_ASSIGN(OneshotTimer);
};

// not thread safe.
class RepeatTimer : public detail::TimerAbstruct<true> {
  public:
    RepeatTimer(uint64 micro_secs, Closure* closure)
        : detail::TimerAbstruct<false>(micro_secs, closure) {
    }
    RepeatTimer(uint64 micro_secs, EventManager* ev_mgr, Closure* closure)
        : detail::TimerAbstruct<false>(micro_secs, ev_mgr, closure) {
    }
    virtual ~RepeatTimer() {
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(RepeatTimer);
};
}
#endif /* TIMER_H_ */
